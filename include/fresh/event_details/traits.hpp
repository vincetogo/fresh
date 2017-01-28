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
            using mutex_type = std::recursive_mutex;
        };
        
        template <>
        struct event_traits<false>
        {
            using mutex_type = std::null_mutex;
        };
    }
}

#endif /* traits_h */
