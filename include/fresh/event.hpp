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
        class ev_connection_base;
     
        template <class Impl, class FnType, template <class T> class Alloc>
        class event_base;

        class event_interface;

        class source_base;
        
        template<class Fn>
        class source;
    }
    
    class ev_connection;
    
    template <class FnType, template <class T> class Alloc = std::allocator>
    class event;
}

class fresh::event_details::event_interface
{
    friend ev_connection;
    
    virtual void close(ev_connection* cnxn) = 0;
};

template <class Impl>
class fresh::event_details::ev_connection_base
{
    template <class Fn>
    friend class fresh::event_details::source;
    
    
private:
    
    void clear()
    {
        ((Impl*)this)->clear();
    }
};

class fresh::event_details::source_base
{
protected:
    friend ev_connection;
    
    ev_connection_base<ev_connection>* _connection = nullptr;
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
    friend class fresh::ev_connection;
    
    source(std::function<Fn> fn) :
        _fn(std::make_unique<std::function<Fn>>(fn))
    {
    }
    
    source& operator = (source&& other)
    {
        _fn = std::move(other._fn);
        return *this;
    }
    
    std::unique_ptr<std::function<Fn>>  _fn;
};

class fresh::ev_connection :
    public event_details::ev_connection_base<ev_connection>
{
public:
    
    ev_connection(ev_connection&& other)
    {
        FRESH_NAMED_LOCK_GUARD(lock1, _mutex);
        FRESH_NAMED_LOCK_GUARD(lock2, other._mutex);
        
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
    
    ~ev_connection()
    {
        if (!!_event)
        {
            _event->close(this);
        }
    }
    
private:
    template <class Impl, class FnType, template <class T> class Alloc>
    friend class fresh::event_details::event_base;
    
    friend class ev_connection_base<ev_connection>;

    template<class Fn>
    ev_connection(void* fn, event_details::event_interface* e,
                  event_details::source<Fn>* src) :
        _event(e),
        _fn(fn),
        _source(src)
    {
        src->_connection = this;
    }
    
    ev_connection& operator= (const ev_connection& other) = delete;
    
    void clear()
    {
        FRESH_LOCK_GUARD(_mutex);
        
        _event = nullptr;
        _fn = nullptr;
    }
    
    event_details::event_interface* _event;
    void*                           _fn;
    event_details::source_base*     _source;
    std::recursive_mutex            _mutex;
};

template <class Impl, template <class S> class Alloc, class Result, class... Args>
class fresh::event_details::event_base<Impl, Result(Args...), Alloc> :
    public event_interface
{
public:
    
    using source_type = source<Result(Args...)>;
    
    ev_connection connect(const std::function<Result(Args...)>& fn)
    {
        FRESH_LOCK_GUARD(_mutex);
        
        source_type source(fn);
        void* key = source._fn.get();
        
        auto& value = _sources[key];
        
        value = std::move(source);
        
        return ev_connection(key, this, &value);
    }
    
protected:
    
    template<class Callable>
    void
    do_call(Callable forEach, Args... args)
    {
        //FRESH_LOCK_GUARD(_mutex);
        
        auto old_can_clean = _can_clean;
        
        _can_clean = false;
        
        for (auto& key_source : _sources)
        {
            auto& source = key_source.second;
            
            //results.push_back((*source._fn)(args...));
            ((Impl*)this)->make_call(forEach, *source._fn, args...);
        }
        
        _can_clean = old_can_clean;
        clean();
    }
    
private:
    
    void clean()
    {
        if (!_can_clean) return;
        
        for (auto cnxn : _invalid_connections)
        {
            auto pos = _sources.find(cnxn->_fn);
            
            if (pos != _sources.end())
            {
                _sources.erase(pos);
                
                cnxn->_fn = nullptr;
                cnxn->_event = nullptr;
            }
        }
        
        _invalid_connections.clear();
    }
    
    void close(ev_connection* cnxn) override
    {
        FRESH_LOCK_GUARD(_mutex);
        
        _invalid_connections.insert(cnxn);
        
        clean();
    }
    
protected:
    using connection_set_type =
        std::set
            <ev_connection*, std::less<ev_connection*>, Alloc<ev_connection*>>;

    using source_map_type =
        std::map<void*, source_type, std::less<void*>, Alloc<source_type>>;
    
    bool                    _can_clean = true;
    connection_set_type     _invalid_connections;
    std::recursive_mutex    _mutex;
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
        FRESH_LOCK_GUARD(base::_mutex);
        
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
                   const std::function<Result(Args...)>& fn,
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
        FRESH_LOCK_GUARD(base::_mutex);
        
        base::do_call(nullptr, args...);
    }
    
private:
    
    friend base;
    
    void make_call(std::nullptr_t,
                   const std::function<void(Args...)>& fn,
                   Args... args)
    {
        fn(args...);
    }
};

#endif
