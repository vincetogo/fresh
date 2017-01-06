//
//  property.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_hpp
#define fresh_property_hpp

#include "property_details/writable_field.hpp"
#include "property_details/dynamic_impl.hpp"

namespace fresh
{
    // signal types
    
    struct default_signal
    {
        using connection_type = connection;
        using signal_type = event<void()>;
        
        template<class... Args>
        static connection_type
        connect(signal_type& sig, Args... args)
        {
            return sig.connect(args...);
        }
    };
    
    struct null_signal
    {
    };
    
    struct read_only
    {
        using event_traits = null_signal;
    };
    
    template <class OwnerType, class PropertyTraits = null_signal>
    struct dynamic
    {
        using owner_type = OwnerType;
        using event_traits = PropertyTraits;
    };
    
    template <class PropertyTraits = null_signal>
    struct writable
    {
        using event_traits = PropertyTraits;
    };
    
    template <class WriterType,
              class PropertyTraits = null_signal>
    struct writable_by
    {
    public:
        using writer = WriterType;
        
        using event_traits = PropertyTraits;
    };
    
    using light = writable<null_signal>;

    template <class OwnerType>
    using light_dynamic = dynamic<OwnerType, null_signal>;
    
    using light_writable = writable<null_signal>;
    
    template <class WriterType>
    using light_writable_by = writable_by<WriterType, null_signal>;
    
    template<class PropertyType>
    struct function_params;
    
    template<class Owner, class PropertyTraits>
    struct function_params<dynamic<Owner, PropertyTraits>>
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
    
    template<class PropertyTraits>
    struct function_params<writable<PropertyTraits>> :
        public field_function_params
    {
    };
    
    template<class W, class PropertyTraits>
    struct function_params<writable_by<W, PropertyTraits>> :
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
    class property<T, read_only, Getter, Setter>
    {
        static_assert(!Getter && !Setter, "Invalid read_only property declaration");
    public:
        
        property(T value) :
            _value(value)
        {
        }
        
        property(const property& other) :
            _value(other())
        {
        }
        
        property(std::nullptr_t) :
            _value(nullptr)
        {
        }
        
        const T& operator () () const
        {
            return _value;
        }
        
        bool operator == (T other) const
        {
            return _value == other;
        }
        
        bool operator != (T other) const
        {
            return !operator==(other);
        }
        
    private:
        
        const T _value;
    };
    
    template <class T,
              class PropertyTraits,
              bool Getter,
              bool Setter>
    class property<T, writable<PropertyTraits>, Getter, Setter> :
        public property_details::writable_field
            <T, PropertyTraits, property_details::any_class>
    {
        static_assert(!Getter && !Setter,
                      "Invalid writable property declaration");
        
    public:
        using base = property_details::writable_field
            <T, PropertyTraits, property_details::any_class>;
        
        using base::base;
        using base::operator=;
    };
    
    template <class T,
              class WriterType,
              class PropertyTraits,
              bool Getter, bool Setter>
    class property<T, writable_by<WriterType, PropertyTraits>, Getter, Setter> :
        public property_details::writable_field<T, PropertyTraits, WriterType>
    {
        static_assert(!Getter && !Setter,
                      "Invalid writable_by property declaration");
        
    public:
        using base =
            property_details::writable_field<T, PropertyTraits, WriterType>;
        
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
        
        friend WriterType;
    };
}

#endif
