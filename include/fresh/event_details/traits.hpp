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

    template <class... Args>
    struct is_pass_by_copy;
    
    template <class T>
    struct is_pass_by_copy<T>
    {
        static const bool value = true;
    };
    
    template <>
    struct is_pass_by_copy<void>
    {
        static const bool value = true;
    };
    
    template <class T>
    struct is_pass_by_copy<T&>
    {
        static const bool value = false;
    };
    
    template <>
    struct is_pass_by_copy<>
    {
        static const bool value = true;
    };
    
    template <class First, class... Rest>
    struct is_pass_by_copy<First, Rest...>
    {
        static const bool value = is_pass_by_copy<First>::value &&
        is_pass_by_copy<Rest...>::value;
    };
    
    template <class FnType>
    struct is_thread_safe_function_type;
    
    template <class ReturnType, class... Args>
    struct is_thread_safe_function_type<ReturnType(Args...)>
    {
        static const bool value = is_pass_by_copy<ReturnType>::value &&
            is_pass_by_copy<Args...>::value;
    };

    
}

#endif /* traits_h */
