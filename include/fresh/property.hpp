//
//  property.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_hpp
#define fresh_property_hpp

#include "property_details/field_properties.hpp"
#include "property_details/dynamic_properties.hpp"

namespace fresh
{
    // signal types
    
    template <class SignalType, class ConnectionType>
    struct signal_info
    {
        using connection_type = ConnectionType;
        using signal_type = SignalType;
    };
    
    struct default_signal : signal_info<signal<void()>, connection>
    {
        template<class... Args>
        static connection
        connect(signal_type& sig, const std::function<void()>& fn, Args... args)
        {
            return sig.connect(fn);
        }
        
        static void
        disconnect(connection& cnxn)
        {
            cnxn.disconnect();
        }
        
        static void
        disconnect_all(signal_type& sig)
        {
            sig.disconnect_all();
        }
    };
    
    struct null_signal
    {
    };
    
    // assignment tests for writable field-type properties
    
    template <class T>
    struct assign_always
    {
        using value_type = typename property_details::format<T>::value_type;

        constexpr static bool test(const value_type&, const value_type&)
        {
            return true;
        }
    };
    
    template <class T>
    struct assign_different
    {
        using value_type = typename property_details::format<T>::value_type;
        
        static bool test(const value_type& currValue, const value_type& newValue)
        {
            return currValue != newValue;
        }
    };
    
    template <class T>
    struct assign_never
    {
        using value_type = typename property_details::format<T>::value_type;
        
        constexpr static bool test(const value_type&, const value_type&)
        {
            return false;
        }
    };
    
    struct read_only
    {
        template <class T>
        using assignment_test = assign_never<T>;
        
        using signal_info = null_signal;
    };
    
    template <class OwnerType, class SignalInfo = default_signal>
    struct dynamic
    {
        using owner_type = OwnerType;
        using signal_info = SignalInfo;
    };
    
    template <template <class T> class AssignmentTest = assign_always,
              class SignalInfo = default_signal>
    struct writable
    {
        template <class T>
        using assignment_test = AssignmentTest<T>;
        
        using signal_info = SignalInfo;
    };
    
    template <class W,
              template <class T> class AssignmentTest = assign_always,
              class SignalInfo = default_signal>
    struct writable_by
    {
    public:
        using writer = W;
        
        template <class T>
        using assignment_test = AssignmentTest<T>;
        
        using signal_info = SignalInfo;
    };
    
    using light = writable<assign_always, null_signal>;

    template <class OwnerType>
    using light_dynamic = dynamic<OwnerType, null_signal>;
    
    template <template <class T> class AssignmentTest = assign_always>
    using light_writable = writable<AssignmentTest, null_signal>;
    
    template <class W, template <class T> class AssignmentTest = assign_always>
    using light_writable_by = writable_by<W, AssignmentTest, null_signal>;
    
    template<class PropertyType>
    struct function_params;
    
    template<class Owner, class SignalInfo>
    struct function_params<dynamic<Owner, SignalInfo>>
    {
        template <class T>
        using getter_type = typename property_details::getter<T, Owner>::type;
        
        template <class T>
        using setter_type = typename property_details::setter<T, Owner>::type;
        
        constexpr static auto default_value() -> std::nullptr_t
        {
            return nullptr;
        }
    };
    
    struct field_function_params
    {
        template <class T>
        using getter_type = bool;
        
        template <class T>
        using setter_type = bool;
        
        constexpr static int default_value()
        {
            return false;
        }
    };
    
    template<>
    struct function_params<read_only> :
        public field_function_params
    {
    };
    
    template<template <class T2> class AssignmentTest, class SignalInfo>
    struct function_params<writable<AssignmentTest, SignalInfo>> :
        public field_function_params
    {
    };
    
    template<class W, template <class T2> class AssignmentTest, class SignalInfo>
    struct function_params<writable_by<W, AssignmentTest, SignalInfo>> :
        public field_function_params
    {
    };
    
    template <class T,
              class PropertyType = writable<>,
              typename function_params<PropertyType>::template getter_type<T> Getter =
                function_params<PropertyType>::default_value(),
              typename function_params<PropertyType>::template setter_type<T> Setter =
                function_params<PropertyType>::default_value()>
    class property :
        public property_details::dynamic_impl<
            T,
            PropertyType,
            std::integral_constant
                <typename function_params<PropertyType>::template getter_type<T>, Getter>,
            std::integral_constant
                <typename function_params<PropertyType>::template setter_type<T>, Setter>>
    {
    public:
        using base = property_details::dynamic_impl<
            T,
            PropertyType,
            std::integral_constant
                <typename function_params<PropertyType>::template getter_type<T>, Getter>,
            std::integral_constant
                <typename function_params<PropertyType>::template setter_type<T>, Setter>>;
        
        using base::base;
        using base::operator=;
    };
    
    template <class T, bool Getter, bool Setter>
    class property<T, read_only, Getter, Setter> :
        public property_details::readable_field<T>
    {
        static_assert(!Getter && !Setter, "Invalid read_only property declaration");
    public:
        
        using base = property_details::readable_field<T>;
        using base::base;
        
        property& operator= (const property& rhs) = delete;
    };
    
    template <class T,
              template <class T2> class AssignmentTest,
              class SignalInfo,
              bool Getter,
              bool Setter>
    class property<T, writable<AssignmentTest, SignalInfo>, Getter, Setter> :
        public property_details::writable_field<T, &AssignmentTest<T>::test, SignalInfo, property_details::any_class>
    {
        static_assert(!Getter && !Setter, "Invalid writable property declaration");
        
    public:
        using base = property_details::writable_field<T, &AssignmentTest<T>::test, SignalInfo, property_details::any_class>;
        
        using base::base;
        using base::operator=;
    };
    
    template <class T,
              class W,
              template <class T2> class AssignmentTest,
              class SignalInfo,
              bool Getter, bool Setter>
    class property<T, writable_by<W, AssignmentTest, SignalInfo>, Getter, Setter> :
        public property_details::writable_field<T, &AssignmentTest<T>::test, SignalInfo, W>
    {
        static_assert(!Getter && !Setter, "Invalid writable_by property declaration");
        
    public:
        using base = property_details::writable_field<T, &AssignmentTest<T>::test, SignalInfo, W>;
        
        using base::base;
        
    protected:
        
        using base::operator=;
        using base::operator+=;
        using base::operator++;
        using base::operator-=;
        using base::operator--;
        
        T& get_mutable()
        {
            return base::_value;
        }
        
    private:
        
        friend W;
    };
}

#endif
