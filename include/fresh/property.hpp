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
    
#ifdef __cpp_template_auto

    template <class T,
              class PropertyType = writable<>,
              auto... Args>
    class property;
    
    template <class T,
        class Owner,
        class PropertyTraits,
        typename property_details::getter<T, Owner>::type Getter>
    class property<T, dynamic<Owner, PropertyTraits>, Getter> :
        public property_details::gettable<T, Owner, PropertyTraits, Getter>
    {
    public:
        
        using base = property_details::
            gettable<T, Owner, PropertyTraits, Getter>;
        
        using base::base;
    };
    
    template <class T,
        class Owner,
        class PropertyTraits,
        typename property_details::getter<T, Owner>::type Getter,
        typename property_details::setter<T, Owner>::type Setter>
    class property<T, dynamic<Owner, PropertyTraits>, Getter, Setter> :
        public property_details::
            settable<T, Owner, PropertyTraits, Getter, Setter>
    {
    public:
        
        using base = property_details::
            settable<T, Owner, PropertyTraits, Getter, Setter>;
        
        using base::base;
        using base::operator=;
    };
    
    template <class T, class PropertyType, auto Getter, auto... Rest>
    using dynamic_property
        = property<T, PropertyType, Getter, Rest...>;
    
    
#else
    
    template <class T, class PropertyType = writable<>>
    class property;
    
    template<class PropertyType>
    struct function_params;
    
    template<class Owner, class PropertyTraits>
    struct function_params<dynamic<Owner, PropertyTraits>>
    {
        template <class T>
        using getter_type = typename property_details::getter<T, Owner>::type;
        
        template <class T>
        using setter_type = typename property_details::setter<T, Owner>::type;
    };
    
    template <class T, class PropertyType,
        typename function_params<PropertyType>::template getter_type<T> Getter,
        typename function_params<PropertyType>::template setter_type<T> Setter
            = nullptr>
    class dynamic_property :
        public property_details::dynamic_impl<T, PropertyType,
            std::integral_constant<typename function_params<PropertyType>::template getter_type<T>, Getter>,
            std::integral_constant<typename function_params<PropertyType>::template setter_type<T>, Setter>>
    {
    public:
        using base = property_details::dynamic_impl<T, PropertyType,
            std::integral_constant<typename function_params<PropertyType>::
                template getter_type<T>, Getter>,
            std::integral_constant<typename function_params<PropertyType>::
                template setter_type<T>, Setter>>;
        
        using base::base;
        using base::operator=;
    };
    
#endif
    
    template <class T>
    class property<T, read_only>
    {
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
              class PropertyTraits>
    class property<T, writable<PropertyTraits>> :
        public property_details::writable_field
            <T, PropertyTraits, property_details::any_class>
    {
    public:
        using base = property_details::writable_field
            <T, PropertyTraits, property_details::any_class>;
        
        using base::base;
        using base::operator=;
    };
    
    template <class T,
              class WriterType,
              class PropertyTraits>
    class property<T, writable_by<WriterType, PropertyTraits>> :
        public property_details::writable_field<T, PropertyTraits, WriterType>
    {
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
