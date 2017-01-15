//
//  traits.hpp
//  fresh_tests
//
//  Created by Vince Tourangeau on 1/14/17.
//  Copyright © 2017 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_traits_hpp
#define fresh_property_details_traits_hpp

#include "../threads.hpp"

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
        
        template <class T, class Attributes>
        struct is_atomic
        {
            static const bool value =
                Attributes::thread_safe &&
                std::is_integral<T>::value &&
                !std::is_same<T, bool>::value;
        };
        
        template <class T,
            class Attributes,
            bool = Attributes::thread_safe,
            bool = is_atomic<T, Attributes>::value>
        struct readable_traits
        {
            using mutex_type = fresh::shared_mutex;
            using value_type = T;
        };
        
        template <class T, class Attributes>
        struct readable_traits<T, Attributes, false, false>
        {
            using mutex_type = fresh::null_mutex;
            using value_type = T;
        };
        
        template <class T, class Attributes>
        struct readable_traits<T, Attributes, true, true>
        {
            using mutex_type = fresh::atomic_mutex;
            using value_type = std::atomic<T>;
        };
        
        template <class T, class Attributes, bool = Attributes::thread_safe>
        struct property_traits
        {
            using arg_type      = T;
            using mutex_type    = typename readable_traits<T, Attributes>::mutex_type;
            using result_type   = T;
            using value_type    = typename readable_traits<T, Attributes>::value_type;
        };
        
        template <class T, class Attributes>
        struct property_traits<T, Attributes, false>
        {
            using arg_type      = const T&;
            using mutex_type    = typename readable_traits<T, Attributes>::mutex_type;
            using result_type   = T;
            using value_type    = typename readable_traits<T, Attributes>::value_type;
        };
        
        template <class T, class Attributes>
        struct property_traits<T&, Attributes, false>
        {
            using arg_type      = const T&;
            using mutex_type    = typename readable_traits<T, Attributes>::mutex_type;
            using result_type   = const T&;
            using value_type    = typename readable_traits<T, Attributes>::value_type;
        };
        
        template <class T, class Attributes>
        struct property_traits<const T&, Attributes, false>
        {
            using arg_type      = const T&;
            using mutex_type    = typename readable_traits<T, Attributes>::mutex_type;
            using result_type   = const T&;
            using value_type    = typename readable_traits<T, Attributes>::value_type;
        };
        
        template <class T, class Attributes>
        struct property_traits<T&, Attributes, true>
        {
            static_assert(!Attributes::thread_safe,
                          "Thread-safe properties cannot return by reference.");
        };
        
        template <class T, class Attributes>
        struct property_traits<const T&, Attributes, true>
        {
            static_assert(!Attributes::thread_safe,
                          "Thread-safe properties cannot return by reference.");
        };
        
        struct does_not_exist {};
        
        template<typename T, typename Arg> does_not_exist operator==
        (const T&, const Arg&);
        
        template<typename T, typename Arg> does_not_exist operator+
        (const T&, const Arg&);

        template<typename T, typename Arg> does_not_exist operator-
        (const T&, const Arg&);

        template <class T, class Arg = T>
        struct has_add
        {
            static const bool value =
                !std::is_same<decltype(std::declval<T>() + std::declval<Arg>()),
                    does_not_exist>::value;
        };
        
        template <class T, class Arg>
        struct has_add<std::atomic<T>, Arg>
        {
            static const bool value =
                has_add<T>::value;
        };
        
        template <class T, class Arg = T>
        struct has_compare
        {
            static const bool value =
                !std::is_same<decltype(std::declval<T>() == std::declval<Arg>()),
                    does_not_exist>::value;
        };
        
        template <class T, class Arg>
        struct has_compare<std::atomic<T>, Arg>
        {
            static const bool value =
                has_compare<T>::value;
        };
        
        template <class T, class Arg = T>
        struct has_subtract
        {
            static const bool value =
                !std::is_same<decltype(std::declval<T>() - std::declval<Arg>()),
                    does_not_exist>::value;
        };
        
        template <class T, class Arg>
        struct has_subtract<std::atomic<T>, Arg>
        {
            static const bool value =
                has_subtract<T>::value;
        };
    }
}

#endif
