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
    struct event_traits
    {
        using connection_type = ConnectionType;
        using signal_type = SignalType;
    };
    
    struct default_signal : event_traits<event<void()>, ev_connection>
    {
        using connection_type = ev_connection;
        
        template<class... Args>
        static connection_type
        connect(signal_type& sig, const std::function<void()>& fn, Args... args)
        {
            return sig.connect(fn);
        }
        
        static void
        disconnect(connection_type& cnxn)
        {
            //cnxn.disconnect();
        }
        
        static void
        disconnect_all(signal_type& sig)
        {
            //sig.disconnect_all();
        }
    };
    
    struct null_signal
    {
    };
    
    struct read_only
    {
        using event_traits = null_signal;
    };
    
    template <class OwnerType, class EventTraits = default_signal>
    struct dynamic
    {
        using owner_type = OwnerType;
        using event_traits = EventTraits;
    };
    
    template <class EventTraits = default_signal>
    struct writable
    {
        using event_traits = EventTraits;
    };
    
    template <class WriterType,
              class EventTraits = default_signal>
    struct writable_by
    {
    public:
        using writer = WriterType;
        
        using event_traits = EventTraits;
    };
    
    using light = writable<null_signal>;

    template <class OwnerType>
    using light_dynamic = dynamic<OwnerType, null_signal>;
    
    using light_writable = writable<null_signal>;
    
    template <class WriterType>
    using light_writable_by = writable_by<WriterType, null_signal>;
    
    template<class PropertyType>
    struct function_params;
    
    template<class Owner, class EventTraits>
    struct function_params<dynamic<Owner, EventTraits>>
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
    
    template<class EventTraits>
    struct function_params<writable<EventTraits>> :
        public field_function_params
    {
    };
    
    template<class W, class EventTraits>
    struct function_params<writable_by<W, EventTraits>> :
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
              class EventTraits,
              bool Getter,
              bool Setter>
    class property<T, writable<EventTraits>, Getter, Setter> :
        public property_details::writable_field
            <T, EventTraits, property_details::any_class>
    {
        static_assert(!Getter && !Setter,
                      "Invalid writable property declaration");
        
    public:
        using base = property_details::writable_field
            <T, EventTraits, property_details::any_class>;
        
        using base::base;
        using base::operator=;
    };
    
    template <class T,
              class WriterType,
              class EventTraits,
              bool Getter, bool Setter>
    class property<T, writable_by<WriterType, EventTraits>, Getter, Setter> :
        public property_details::writable_field<T, EventTraits, WriterType>
    {
        static_assert(!Getter && !Setter,
                      "Invalid writable_by property declaration");
        
    public:
        using base =
            property_details::writable_field<T, EventTraits, WriterType>;
        
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
