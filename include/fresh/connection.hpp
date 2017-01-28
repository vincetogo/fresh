//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_connection_hpp
#define fresh_connection_hpp

#include "event_details/connection_base.hpp"
#include "event_details/source_base.hpp"
#include "event_interface.hpp"
#include "threads.hpp"

#include <cassert>

namespace fresh
{
    template <bool ThreadSafe = false>
    class connection;
    
    template <class Impl, class FnType, bool ThreadSafe, template <class T> class Alloc>
    class event_base;
}

template <bool ThreadSafe>
class fresh::connection :
    public event_details::connection_base<connection<ThreadSafe>>
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
    template <class Impl, class FnType, bool ThreadSafeBase, template <class T> class Alloc>
    friend class fresh::event_base;
    
    friend class event_details::connection_base<connection<ThreadSafe>>;
    
    template<class Fn>
    connection(void* fn, event_interface<ThreadSafe>* e,
               event_details::source<Fn, ThreadSafe>* src) :
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
    
    event_interface<ThreadSafe>*            _event;
    void*                                   _fn;
    event_details::source_base<ThreadSafe>* _source;
    mutable std::mutex                      _mutex;
};

#endif
