//
//  traits.hpp
//  fresh_tests
//
//  Created by Vince Tourangeau on 1/14/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_traits_hpp
#define fresh_property_details_traits_hpp

namespace fresh
{
    struct null_signal;
    
    namespace property_details
    {
        template <class Attributes>
        struct has_event
        {
            static const bool value =
                !std::is_same<typename Attributes::event_type, null_signal>::value;
        };
    }
}

#endif
