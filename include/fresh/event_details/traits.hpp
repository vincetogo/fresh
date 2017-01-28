//
//  traits.hpp
//  Prog
//
//  Created by Vince Tourangeau on 1/28/17.
//  Copyright © 2017 Cryogenic Head. All rights reserved.
//

#ifndef fresh_event_details_traits_hpp
#define fresh_event_details_traits_hpp

#include "../threads.hpp"

namespace fresh
{
    namespace event_details
    {
        template <bool ThreadSafe = false>
        struct event_traits
        {
            using connection_mutex_type = std::mutex;
            using event_mutex_type      = std::recursive_mutex;
            using function_mutex_type   = fresh::shared_mutex;
        };
        
        template <>
        struct event_traits<false>
        {
            using connection_mutex_type = fresh::null_mutex;
            using event_mutex_type      = fresh::null_mutex;
            using function_mutex_type   = fresh::null_mutex;
        };
    }
}

#endif /* traits_h */
