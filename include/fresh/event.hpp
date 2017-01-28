//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_hpp
#define fresh_event_hpp

#include "connection.hpp"
#include "event_details/source.hpp"
#include "event_details/traits.hpp"
#include "threads.hpp"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace fresh
{
    template <class FnType, bool ThreadSafe = false,
        template <class T> class Alloc = std::allocator>
    class event;
    
    template <class Impl,
        class FnType,
        bool ThreadSafe,
        template <class T> class Alloc>
    class event_base;
}

template <class Impl,
    bool ThreadSafe,
    template <class S> class Alloc,
    class Result,
    class... Args>
class fresh::event_base<Impl, Result(Args...), ThreadSafe, Alloc> :
    public event_interface<ThreadSafe>
{
public:
    
    using connection_type = connection<ThreadSafe>;
    using mutex_type =
        typename event_details::event_traits<ThreadSafe>::event_mutex_type;
    using lock_type = std::lock_guard<mutex_type>;
    using source_type = event_details::source<Result(Args...), ThreadSafe>;
    
    connection_type connect(const std::function<Result(Args...)>& fn)
    {
        lock_type lock(_mutex);
        
        source_type source(fn);
        void* key = source._fn.get();
        
        auto& value = _sources[key];
        
        value = std::move(source);
        
        return connection_type(key, this, &value);
    }
    
protected:
    
    template<class Callable>
    void
    call(Callable forEach, Args... args)
    {
        std::vector<decltype(source_type::_fn)> fns;
        
        {
            lock_type lock(_mutex);
            
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
            ((Impl*)this)->call_function(forEach, *fn, args...);
        }
        
        _can_clean = old_can_clean;
        
        lock_type lock(_mutex);
        clean();
    }
    
    mutex_type          _mutex;
    static const bool   _clean_guard = true;
    
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
    
    void close(connection_type& cnxn) override
    {
        lock_type lock(_mutex);
        
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
            <connection_type, std::less<connection_type>, Alloc<connection_type>>;
    
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

template<bool ThreadSafe, template <class T> class Alloc, class Result, class... Args>
class fresh::event<Result(Args...), ThreadSafe, Alloc> :
    public event_base
        <event<Result(Args...), ThreadSafe, Alloc>, Result(Args...), ThreadSafe, Alloc>
{
public:
    
    using base =
        event_base
            <event<Result(Args...), ThreadSafe, Alloc>, Result(Args...), ThreadSafe, Alloc>;
    
    using base::base;
    
    auto operator()(Args... args) -> std::vector<Result, Alloc<Result>>
    {
        std::vector<Result, Alloc<Result>> results;
        
        base::call(
            [&](const Result& result)
            {
                results.push_back(result);
            });
        
        return results;
    }
    
private:
    
    friend base;
    
    void call_function(const std::function<void(const Result&)>& forEach,
                   const event_details::event_function<Result(Args...), ThreadSafe>& fn,
                   Args... args)
    {
        forEach(fn(args...));
    }
};

template<bool ThreadSafe, template <class T> class Alloc, class... Args>
class fresh::event<void(Args...), ThreadSafe, Alloc> :
    public event_base
        <event<void(Args...), ThreadSafe, Alloc>, void(Args...), ThreadSafe, Alloc>
{
public:
    
    using base =
        event_base
            <event<void(Args...), ThreadSafe, Alloc>, void(Args...), ThreadSafe, Alloc>;
    
    using base::base;
    
    void operator()(Args... args)
    {
        base::call(nullptr, args...);
    }
    
private:
    
    friend base;
    
    void call_function(std::nullptr_t,
                   const event_details::event_function<void(Args...), ThreadSafe>& fn,
                   Args... args)
    {
        fn(args...);
    }
};

#endif
