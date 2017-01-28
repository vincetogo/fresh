//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_details_source_base_hpp
#define fresh_event_details_source_base_hpp

namespace fresh
{
    namespace event_details
    {
        template<class Impl>
        class connection_base;
     
        template<bool ThreadSafe>
        class source_base;
    }
    
    template <bool ThreadSafe>
    class connection;
}

template<bool ThreadSafe>
class fresh::event_details::source_base
{
protected:
    friend connection<ThreadSafe>;
    
    connection_base<connection<ThreadSafe>>* _connection = nullptr;
};

#endif
