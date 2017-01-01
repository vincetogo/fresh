//
// signal.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_signal_hpp
#define fresh_signal_hpp

#include "threads.hpp"

#include <assert.h>

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace fresh
{
    class connection;
    class signal_interface;
    
    namespace signal_details
    {
        class connection_data
        {
        public:
            connection_data(void* function, signal_interface* signal) :
                _function(function),
                _mutex(),
                _signal(signal),
                _valid(true)
            {}
            
        private:
            friend connection;

            void*               _function;
            mutable std::mutex  _mutex;
            signal_interface*   _signal;
            bool                _valid;
        };
    }
    
    class connection
    {
    public:
        connection() :
            _details(),
            _mutex(),
            _lock(_mutex, std::defer_lock)
        {}
        
        connection(void* function, signal_interface* signal) :
            _details(),
            _mutex(),
            _lock(_mutex, std::defer_lock)
        {
            _details = std::make_shared<signal_details::connection_data>(function, signal);
        }
        
        connection(const connection& rhs) :
            _details(rhs._details),
            _mutex(),
            _lock(_mutex, std::defer_lock)
        {}
        
        connection& operator=(const connection& rhs)
        {
            if (&rhs == this) return *this;
            
            FRESH_NAMED_LOCK_GUARD(lock1, lock());
            FRESH_NAMED_LOCK_GUARD(lock2, rhs.lock());
            
            _details = rhs._details;
            
            return *this;
        }
        
        bool operator < (const connection& rhs) const
        {
            if (&rhs == this) return false;
            
            FRESH_NAMED_LOCK_GUARD(lock1, lock());
            FRESH_NAMED_LOCK_GUARD(lock2, rhs.lock());
            
            return _details < rhs._details;
        }
        
        void disconnect();
        
        bool operator == (const connection& rhs) const
        {
            if (&rhs == this) return true;
            
            FRESH_NAMED_LOCK_GUARD(lock1, lock());
            FRESH_NAMED_LOCK_GUARD(lock2, rhs.lock());
            
            return _details == rhs._details;
        }
        
        template <typename slot>
        const slot& function() const
        {
            // because this returns a reference, it is only valid while
            // the mutex remains locked by the caller
 
            assert(lock().owns_lock());
            return *reinterpret_cast<slot*>(_details->_function);
        }
        
        bool is_valid() const
        {
            assert(lock().owns_lock());
            return _details ? _details->_valid : false;
        }
        
        std::unique_lock<std::mutex>&  lock() const
        {
            return _lock;
        }
        
        template <typename slot>
        void clear()
        {
            FRESH_LOCK_GUARD(lock());
            FRESH_NAMED_LOCK_GUARD(lock2, _details->_mutex);
            
            delete(reinterpret_cast<slot*>(_details->_function));
            
            _details->_function = nullptr;
        }
        
    private:
        
        std::shared_ptr<signal_details::connection_data>    _details;
        mutable std::mutex                                  _mutex;
        mutable std::unique_lock<std::mutex>                _lock;
    };
    
    class signal_interface
    {
    public:
        virtual ~signal_interface() {};
        
    private:
        
        friend void connection::disconnect();
        virtual void disconnect(const connection& cnxn) = 0;
    };
    
    inline void connection::disconnect()
    {
        FRESH_LOCK_GUARD(lock());
        
        if (is_valid())
        {
            _details->_valid = false;
            
            if (_details->_signal)
            {
                _details->_signal->disconnect(*this);
            }
        }
    }
    
    namespace signal_details
    {
        template <class R, class... Arg>
        struct caller
        {
            using slot = std::function<R(Arg...)>;
            
            template <class Callable>
            static void call(connection cnxn, Callable forCall, Arg... args)
            {
                FRESH_LOCK_GUARD(cnxn.lock());
                
                if (cnxn.is_valid())
                {
                    forCall(cnxn.template function<slot>()(args...));
                }
            }
        };
        
        template <class... Arg>
        struct caller<void, Arg...>
        {
            using slot = std::function<void(Arg...)>;
            
            template <class Callable>
            static void call(connection cnxn, Callable forEach, Arg... args)
            {
                FRESH_LOCK_GUARD(cnxn.lock());
                
                if (cnxn.is_valid())
                {
                    cnxn.template function<slot>()(args...);
                }
            }
        };
        
        template <>
        struct caller<void>
        {
            using slot = std::function<void()>;
            
            template <class Callable>
            static void call(connection cnxn, Callable forEach)
            {
                std::thread thread(do_call, cnxn);
                forEach(thread);
            }
            
            static void do_call(connection cnxn)
            {
                FRESH_LOCK_GUARD(cnxn.lock());
                
                if (cnxn.is_valid())
                {
                    cnxn.template function<slot>()();
                }
            }
        };
    }
    
    template <typename T>
    class signal_base;
    
    template <typename R, typename... Arg>
    class signal_base<R(Arg...)> : public signal_interface
    {
    public:
        using slot = std::function<R(Arg...)>;
        
        signal_base();
        signal_base(const signal_base& other) = delete;
        
        virtual ~signal_base();
        
        connection connect(const slot& fn);
        
        bool connected() const;
        
        void disconnect_all();
        
    protected:
        
        template<class Callable>
        void call(Callable forEach, Arg... args);
        
    private:
        
        friend void connection::disconnect() const;
        
        void clean();
        void disconnect(const connection& cnxn);
        
        bool                    _canClean = true;
        std::vector<connection> _invalidSlots;
        mutable std::mutex      _mutex;
        std::vector<connection> _slots;
    };
    
    template <typename R, typename... Arg>
    signal_base<R(Arg...)>::signal_base() :
        _slots()
    {
    }
    
    template <typename R, typename... Arg>
    signal_base<R(Arg...)>::~signal_base()
    {
        disconnect_all();
    }
    
    template <typename R, typename... Arg>
    connection
    signal_base<R(Arg...)>::connect(const slot& fn)
    {
        clean();
        
        slot* fnCopy = new slot(fn);
        
        connection result(fnCopy, this);
        
        {
            FRESH_LOCK_GUARD(_mutex);
            
            _slots.push_back(result);
        }
        
        return result;
    }
    
    template <typename R, typename... Arg>
    bool
    signal_base<R(Arg...)>::connected() const
    {
        FRESH_LOCK_GUARD(_mutex);
        
        return !_slots.empty();
    }
    
    template <typename R, typename... Arg>
    void
    signal_base<R(Arg...)>::disconnect_all()
    {
        bool oldCanClean;
        std::vector<connection> cnxns;
        
        {
            FRESH_LOCK_GUARD(_mutex);
            
            for (const auto& cnxn : _slots)
            {
                cnxns.push_back(cnxn);
            }
            
            oldCanClean = _canClean;
            _canClean = false;
        }
        
        for (auto& cnxn : cnxns)
        {
            cnxn.disconnect();
        }
        
        {
            FRESH_LOCK_GUARD(_mutex);
            _canClean = oldCanClean;
        }
        
        clean();
    }
    
    template <typename R, typename... Arg>
    template<class Callable>
    void
    signal_base<R(Arg...)>::call(Callable forEach, Arg... args)
    {
        std::vector<connection> slots;
        
        bool oldCanClean;
        
        {
            FRESH_LOCK_GUARD(_mutex);
            slots = _slots;
            
            oldCanClean = _canClean;
            _canClean = false;
        }
        
        for (auto& cnxn : slots)
        {
            signal_details::caller<R, Arg...>::call(cnxn, forEach, args...);
        }
        
        {
            FRESH_LOCK_GUARD(_mutex);
            _canClean = oldCanClean;
        }
        
        clean();
    }
    
    template <typename R, typename... Arg>
    void
    signal_base<R(Arg...)>::clean()
    {
        FRESH_LOCK_GUARD(_mutex);

        if (!_canClean) return;
        
        for (auto& slot : _invalidSlots)
        {
            _slots.erase(std::find(_slots.begin(), _slots.end(), slot));
        }
    
        for (auto& cnxn : _invalidSlots)
        {
            cnxn.template clear<slot>();
        }
        
        _invalidSlots.clear();
    }
    
    template <typename R, typename... Arg>
    void
    signal_base<R(Arg...)>::disconnect(const connection& cnxn)
    {
        assert(!cnxn.is_valid());
        
        {
            FRESH_LOCK_GUARD(_mutex);
            _invalidSlots.push_back(cnxn);
        }
        
        clean();
    }
    
    template <typename T>
    class signal;
    
    template <typename R, typename... Arg>
    class signal<R(Arg...)> : public signal_base<R(Arg...)>
    {
        using base = signal_base<R(Arg...)>;
        
    public:

        using slot = std::function<R(Arg...)>  ;

        std::vector<R> operator()(Arg... args);
    };
    
    template <class R, class... Arg>
    std::vector<R>
    signal<R(Arg...)>::operator()(Arg... args)
    {
        std::vector<R> result;
        
        auto forEach =
        [&](const R& value)
        {
            result.push_back(value);
        };
        
        base::call(forEach, args...);
        
        return result;
    }
    
    template <typename... Arg>
    class signal<void(Arg...)> : public signal_base<void(Arg...)>
    {
        using base = signal_base<void(Arg...)>;
        
    public:
        
        using slot = std::function<void(Arg...)>;
        
        void operator()(Arg... args);
    };
    
    template <typename... Arg>
    void
    signal<void(Arg...)>::operator()(Arg... args)
    {
        base::call(nullptr, args...);
    }
    
    template <>
    class signal<void()> : public signal_base<void()>
    {
        using base = signal_base<void()>;
        
    public:
        
        using slot = std::function<void()>;
        
        void operator()();
    };
    
    inline void
    signal<void()>::operator()()
    {
        std::vector<std::thread> threads;
        
        base::call(
           [&](std::thread& thread)
           {
               threads.push_back(std::move(thread));
           });
        
        for (auto& thread : threads)
        {
            thread.join();
        }
    }
}

#endif
