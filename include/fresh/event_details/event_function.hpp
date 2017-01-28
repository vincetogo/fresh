//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_details_event_function_hpp
#define fresh_event_details_event_function_hpp

#include "source_base.hpp"
#include "../threads.hpp"

#include <cassert>
#include <functional>

namespace fresh
{
    namespace event_details
    {
        template<class Fn>
        class event_function_base;
        
        template<class Fn>
        class event_function;
    }
}

template<class Fn>
class fresh::event_details::event_function_base
{
public:
    
    event_function_base(const std::function<Fn>& fn) :
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
class fresh::event_details::event_function<Result(Args...)> :
    public event_function_base<Result(Args...)>
{
public:
    
    using base = event_function_base<Result(Args...)>;
    
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
class fresh::event_details::event_function<void(Args...)> :
    public event_function_base<void(Args...)>
{
public:
    
    using base = event_function_base<void(Args...)>;
    
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

#endif
