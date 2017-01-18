//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_hpp
#define fresh_event_hpp

#include "threads.hpp"

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace fresh
{
    namespace event_details
    {
        template<class Impl>
        class connection_base;
     
        template <class Impl, class FnType, template <class T> class Alloc>
        class event_base;

        class event_interface;

        template<class Fn>
        class source;

        class source_base;

        template<class Fn>
        class source_function_base;
        
        template<class Fn>
        class source_function;
    }
    
    class connection;
    
    template <class FnType, template <class T> class Alloc = std::allocator>
    class event;
}

class fresh::event_details::event_interface
{
public:
    virtual ~event_interface() = default;

private:
    friend connection;
    
    virtual void close(connection& cnxn) = 0;
};

template <class Impl>
class fresh::event_details::connection_base
{
    template <class Fn>
    friend class fresh::event_details::source;
    
    
private:
    
    void clear()
    {
        ((Impl*)this)->clear();
    }
};

template<class Fn>
class fresh::event_details::source_function_base
{
public:
    
    source_function_base(const std::function<Fn>& fn) :
    _fn(fn)
    {
    }
    
    void clear()
    {
        fresh::write_lock<fresh::shared_mutex>   lock(_mutex);
        _fn = nullptr;
    }

protected:
    
    std::function<Fn>           _fn;
    mutable fresh::shared_mutex _mutex;
};


template<class Result, class... Args>
class fresh::event_details::source_function<Result(Args...)> :
    public source_function_base<Result(Args...)>
{
public:
    
    using base = source_function_base<Result(Args...)>;
    
    using base::base;
    
    Result
    operator() (Args... args) const
    {
        fresh::read_lock<fresh::shared_mutex>   lock(base::_mutex);
        
        if (base::_fn)
        {
            return base::_fn(args...);
        }
        else
        {
            return Result();
        }
    }
};

template<class... Args>
class fresh::event_details::source_function<void(Args...)> :
    public source_function_base<void(Args...)>
{
public:
    
    using base = source_function_base<void(Args...)>;
    
    using base::base;

    void
    operator() (Args... args) const
    {
        fresh::read_lock<fresh::shared_mutex>   lock(base::_mutex);
        
        if (base::_fn)
        {
            base::_fn(args...);
        }
    }
};

class fresh::event_details::source_base
{
protected:
    friend connection;
    
    connection_base<connection>* _connection = nullptr;
};

template <class Fn>
class fresh::event_details::source : public source_base
{
public:
    
    source()
    {
    }
    
    ~source()
    {
        if (source_base::_connection)
        {
            source_base::_connection->clear();
        }
    }
    
private:
    template <class Impl, class Fn2, template <class S> class Alloc>
    friend class fresh::event_details::event_base;
    friend class fresh::connection;
    
    source(std::function<Fn> fn) :
        _fn(std::make_shared<source_function<Fn>>(fn))
    {
    }
    
    source& operator= (source&& other)
    {
        _fn = std::move(other._fn);
        return *this;
    }
    
    std::shared_ptr<source_function<Fn>>  _fn;
};

class fresh::connection :
    public event_details::connection_base<connection>
{
public:
    
    connection(connection&& other)
    {
        std::lock_guard<std::mutex> lock1(_mutex);
        std::lock_guard<std::mutex> lock2(other._mutex);
        
        _event = other._event;
        _fn = other._fn;
        _source = other._source;
        
        if (_source)
        {
            assert(_source->_connection == &other);
            _source->_connection = this;
        }
        
        other._event = nullptr;
        other._fn = nullptr;
        other._source = nullptr;
    }
    
    ~connection()
    {
        if (!!_event)
        {
            _event->close(*this);
        }
    }
    
    bool operator< (const connection& rhs) const
    {
        std::lock_guard<std::mutex> lock1(_mutex);
        std::lock_guard<std::mutex> lock2(rhs._mutex);
        
        return _fn < rhs._fn;
    }
    
private:
    template <class Impl, class FnType, template <class T> class Alloc>
    friend class fresh::event_details::event_base;
    
    friend class connection_base<connection>;

    template<class Fn>
    connection(void* fn, event_details::event_interface* e,
                  event_details::source<Fn>* src) :
        _event(e),
        _fn(fn),
        _source(src)
    {
        src->_connection = this;
    }
    
    connection& operator= (const connection& other) = delete;
    
    void clear()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        
        _event = nullptr;
        _fn = nullptr;
    }
    
    event_details::event_interface* _event;
    void*                           _fn;
    event_details::source_base*     _source;
    mutable std::mutex              _mutex;
};

template <class Impl, template <class S> class Alloc, class Result, class... Args>
class fresh::event_details::event_base<Impl, Result(Args...), Alloc> :
    public event_interface
{
public:
    
    using source_type = source<Result(Args...)>;
    
    connection connect(const std::function<Result(Args...)>& fn)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        
        source_type source(fn);
        void* key = source._fn.get();
        
        auto& value = _sources[key];
        
        value = std::move(source);
        
        return connection(key, this, &value);
    }
    
protected:
    
    template<class Callable>
    void
    do_call(Callable forEach, Args... args)
    {
        std::vector<decltype(source_type::_fn)> fns;
        
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            for (auto& key_source : _sources)
            {
                auto& source = key_source.second;
                
                fns.push_back(source._fn);
            }
        }
        
        auto old_can_clean = _can_clean;
        _can_clean = _clean_guard;
        
        for (auto& fn : fns)
        {
            ((Impl*)this)->make_call(forEach, *fn, args...);
        }
        
        _can_clean = old_can_clean;
        
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        clean();
    }
    
    std::recursive_mutex    _mutex;
    static const bool       _clean_guard = true;
    
private:
    
    void clean()
    {
        if (!_can_clean) return;
        
        for (auto& cnxn : _invalid_connections)
        {
            _sources.erase(cnxn._fn);
        }
        
        _invalid_connections.clear();
    }
    
    void close(connection& cnxn) override
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        
        auto pos = _sources.find(cnxn._fn);
        
        if (pos != _sources.end())
        {
            pos->second._fn->clear();
            _invalid_connections.insert(std::move(cnxn));
        }
        
        clean();
    }
    
    using connection_set_type =
        std::set
            <connection, std::less<connection>, Alloc<connection>>;
    
    using source_map_alloc_type = Alloc<std::pair<void* const, source_type>>;

    using source_map_type =
        std::map<void*,
            source_type,
            std::less<void*>,
            source_map_alloc_type>;
    
    bool                    _can_clean = true;
    connection_set_type     _invalid_connections;
    source_map_type         _sources;
};

template<template <class T> class Alloc, class Result, class... Args>
class fresh::event<Result(Args...), Alloc> :
    public event_details::event_base
        <event<Result(Args...), Alloc>, Result(Args...), Alloc>
{
public:
    
    using base =
        event_details::event_base
            <event<Result(Args...), Alloc>, Result(Args...), Alloc>;
    
    using base::base;
    
    auto operator()(Args... args) -> std::vector<Result, Alloc<Result>>
    {
        std::vector<Result, Alloc<Result>> results;
        
        base::do_call(
            [&](const Result& result)
            {
                results.push_back(result);
            });
        
        return results;
    }
    
private:
    
    friend base;
    
    void make_call(const std::function<void(const Result&)>& forEach,
                   const event_details::source_function<Result(Args...)>& fn,
                   Args... args)
    {
        forEach(fn(args...));
    }
};

template<template <class T> class Alloc, class... Args>
class fresh::event<void(Args...), Alloc> :
    public event_details::event_base
        <event<void(Args...), Alloc>, void(Args...), Alloc>
{
public:
    
    using base =
        event_details::event_base
            <event<void(Args...), Alloc>, void(Args...), Alloc>;
    
    using base::base;
    
    void operator()(Args... args)
    {
        base::do_call(nullptr, args...);
    }
    
private:
    
    friend base;
    
    void make_call(std::nullptr_t,
                   const event_details::source_function<void(Args...)>& fn,
                   Args... args)
    {
        fn(args...);
    }
};

#endif
