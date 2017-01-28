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
     
        class source_base;
    }
    
    class connection;
}


class fresh::event_details::source_base
{
protected:
    friend connection;
    
    connection_base<connection>* _connection = nullptr;
};

#endif
