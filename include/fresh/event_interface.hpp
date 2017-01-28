//
// event.hpp
//
//  Created by Vincent Tourangeau on 01/03/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_event_interface_hpp
#define fresh_event_interface_hpp

namespace fresh
{
    class connection;
    class event_interface;
}

class fresh::event_interface
{
public:
    virtual ~event_interface() = default;

private:
    friend connection;
    
    virtual void close(connection& cnxn) = 0;
};

#endif
