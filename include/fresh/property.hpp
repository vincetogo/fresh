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
    
    struct null_signal
    {
    };
    
    struct null_connection
    {
    };
    
    template <class Event, class Connection, bool ThreadSafe>
    struct property_attributes
    {
        using connection_type = Connection;
        using event_type = Event;
        static const bool thread_safe = ThreadSafe;
    };
    
    template <bool ThreadSafe>
    struct basic_unobservable :
        public property_attributes<null_signal, null_connection, ThreadSafe>
    {
        using base =
            property_attributes<null_signal, null_connection, ThreadSafe>;
        
        using connection_type = typename base::connection_type;
        using event_type = typename base::event_type;
        
        template<class... Args>
        static connection_type
        connect(event_type& sig, Args... args)
        {
            assert(false); // we should never get here
            
            return null_connection();
        }
    };
    
    template <bool ThreadSafe>
    struct basic_observable :
        public property_attributes<event<void()>, connection, ThreadSafe>
    {
        using base = property_attributes<event<void()>, connection, ThreadSafe>;
        
        using connection_type = typename base::connection_type;
        using event_type = typename base::event_type;
        
        template<class... Args>
        static connection_type
        connect(event_type& sig, Args... args)
        {
            return sig.connect(args...);
        }
    };
    
    // useful aliases
    using observable = basic_observable<false>;
    using thread_safe = basic_unobservable<true>;
    using thread_safe_observable = basic_observable<true>;
    using unobservable = basic_unobservable<false>;

    // what we usually want
    using default_attributes = unobservable;

    struct read_only
    {
        using attributes = default_attributes;
    };
    
    template <class OwnerType, class Attributes = default_attributes>
    struct dynamic
    {
        using owner_type = OwnerType;
        using attributes = Attributes;
    };
    
    template <class Attributes = default_attributes>
    struct writable
    {
        using attributes = Attributes;
    };
    
    template <class WriterType,
              class Attributes = default_attributes>
    struct writable_by
    {
    public:
        using writer = WriterType;
        
        using attributes = Attributes;
    };
    
#ifdef __cpp_template_auto

    template <class T,
              class PropertyType = writable<>,
              auto... Args>
    class property;
    
    template <class T,
        class Owner,
        class Attributes,
        typename property_details::getter<T, Owner>::type Getter>
    class property<T, dynamic<Owner, Attributes>, Getter> :
        public property_details::gettable<T, Owner, Attributes, Getter>
    {
    public:
        
        using base = property_details::
            gettable<T, Owner, Attributes, Getter>;
        
        using base::base;
    };
    
    template <class T,
        class Owner,
        class Attributes,
        typename property_details::getter<T, Owner>::type Getter,
        typename property_details::setter<T, Owner>::type Setter>
    class property<T, dynamic<Owner, Attributes>, Getter, Setter> :
        public property_details::
            settable<T, Owner, Attributes, Getter, Setter>
    {
    public:
        
        using base = property_details::
            settable<T, Owner, Attributes, Getter, Setter>;
        
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
    
    template<class Owner, class Attributes>
    struct function_params<dynamic<Owner, Attributes>>
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
        
        property& operator=(const T other) = delete;
        
    private:
        
        const T _value;
    };
    
    template <class T,
              class Attributes>
    class property<T, writable<Attributes>> :
        public property_details::writable_field
            <T, Attributes, property_details::any_class>
    {
    public:
        using base = property_details::writable_field
            <T, Attributes, property_details::any_class>;
        
        using base::base;
        using base::operator=;
    };
    
    template <class T,
              class WriterType,
              class Attributes>
    class property<T, writable_by<WriterType, Attributes>> :
        public property_details::writable_field<T, Attributes, WriterType>
    {
    public:
        using base =
            property_details::writable_field<T, Attributes, WriterType>;
        
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
