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
#include "threads.hpp"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace fresh
{
    template <class FnType, template <class T> class Alloc = std::allocator>
    class event;
    
    template <class Impl, class FnType, template <class T> class Alloc>
    class event_base;
}

template <class Impl, template <class S> class Alloc, class Result, class... Args>
class fresh::event_base<Impl, Result(Args...), Alloc> :
    public event_interface
{
public:
    
    using source_type = event_details::source<Result(Args...)>;
    
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
    public event_base
        <event<Result(Args...), Alloc>, Result(Args...), Alloc>
{
public:
    
    using base =
        event_base
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
                   const event_details::event_function<Result(Args...)>& fn,
                   Args... args)
    {
        forEach(fn(args...));
    }
};

template<template <class T> class Alloc, class... Args>
class fresh::event<void(Args...), Alloc> :
    public event_base
        <event<void(Args...), Alloc>, void(Args...), Alloc>
{
public:
    
    using base =
        event_base
            <event<void(Args...), Alloc>, void(Args...), Alloc>;
    
    using base::base;
    
    void operator()(Args... args)
    {
        base::do_call(nullptr, args...);
    }
    
private:
    
    friend base;
    
    void make_call(std::nullptr_t,
                   const event_details::event_function<void(Args...)>& fn,
                   Args... args)
    {
        fn(args...);
    }
};

#endif
