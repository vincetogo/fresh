//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_details_connection_base_hpp
#define fresh_event_details_connection_base_hpp

namespace fresh
{
    namespace event_details
    {
        template<class Impl>
        class connection_base;
        
        template <class Fn>
        class source;
    }
}

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

#endif
