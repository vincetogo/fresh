//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_details_source_hpp
#define fresh_event_details_source_hpp

#include "event_function.hpp"
#include "source_base.hpp"

namespace fresh
{
    namespace event_details
    {
        template<class Impl>
        class connection_base;
     
        template<class Fn>
        class source;

        class source_base;
    }
}

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
    friend class fresh::event_base;
    friend class fresh::connection;
    
    source(std::function<Fn> fn) :
        _fn(std::make_shared<event_function<Fn>>(fn))
    {
    }
    
    source& operator= (source&& other)
    {
        _fn = std::move(other._fn);
        return *this;
    }
    
    std::shared_ptr<event_function<Fn>>  _fn;
};

#endif
