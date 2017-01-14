//
//  format.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_format_hpp
#define fresh_property_details_format_hpp

#include "traits.hpp"

namespace fresh
{
    namespace property_details
    {
        template <class T, class Attributes>
        struct format
        {
            using arg_type      = const T&;
            using result_type   = T;
            using value_type    = T;
        };
        
        template <class T, class Attributes>
        struct format<T&, Attributes>
        {
            static_assert(!Attributes::thread_safe,
                "Thread-safe properties cannot return by reference.");
            using arg_type      = const T&;
            using result_type   = const T&;
            using value_type    = T;
        };
        
        template <class T, class Attributes>
        struct format<const T&, Attributes>
        {
            static_assert(!Attributes::thread_safe,
                "Thread-safe properties cannot return by reference.");
            using arg_type      = const T&;
            using result_type   = const T&;
            using value_type    = T;
        };
    }
}

#endif
