//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_details_event_interface_hpp
#define fresh_event_details_event_interface_hpp

namespace fresh
{
    namespace event_details
    {
        template <bool ThreadSafe>
        class event_interface;
    }
    
    template <bool ThreadSafe>
    class connection;
}

template <bool ThreadSafe>
class fresh::event_details::event_interface
{
public:
    virtual ~event_interface() = default;

private:
    friend connection<ThreadSafe>;
    
    virtual void close(connection<ThreadSafe>& cnxn) = 0;
};

#endif
