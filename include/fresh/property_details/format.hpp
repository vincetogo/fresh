//
//  format.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_format_hpp
#define fresh_property_details_format_hpp

namespace fresh
{
    namespace property_details
    {
        template <typename T>
        struct format
        {
            using arg_type      = const T&;
            using result_type   = T;
            using value_type    = T;
        };
        
        template <typename T>
        struct format<T&>
        {
            using arg_type      = const T&;
            using result_type   = const T&;
            using value_type    = T;
        };
        
        template <typename T>
        struct format<const T&>
        {
            using arg_type      = const T&;
            using result_type   = const T&;
            using value_type    = T;
        };
    }
}

#endif
