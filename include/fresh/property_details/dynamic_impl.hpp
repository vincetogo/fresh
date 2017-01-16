//
// property.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_dynamic_properties_hpp
#define fresh_property_details_dynamic_properties_hpp

#include "signaller.hpp"
#include "traits.hpp"

#include <vector>

namespace fresh
{
    template <class OwnerType, class Attributes>
    struct dynamic;
    
    namespace property_details
    {
        template <typename T, typename D>
        struct getter
        {
            using type = T (D::*)() const;
        };
        
        template <typename T, typename D>
        struct getter<T&, D>
        {
            using type = const T& (D::*)() const;
        };
        
        template <typename T, typename D>
        struct setter
        {
            using type = void (D::*)(const T&);
        };
        
        template <typename T, typename D>
        struct setter<T&, D>
        {
            using type = void (D::*)(const T&);
        };
        
        
        template <class Attributes, class Impl, bool = has_event<Attributes>::value>
        class dependent_property
        {
        protected:
            
            template<class... Properties>
            dependent_property(Properties&... properties)
            {
                connect_to_properties(properties...);
            }
            
        private:
            
            template <class Property, class... Properties>
            void
            connect_to_properties(Property& first, Properties&... rest)
            {
                auto cnxn = first.connect(
                    [=]()
                    {
                        ((Impl*)this)->send();
                    });
                
                _propertyConnections.push_back(std::move(cnxn));
                
                connect_to_properties(rest...);
            }
            
            void
            connect_to_properties()
            {
            }
            
            std::vector<typename Attributes::connection_type>
                _propertyConnections;
        };
        
        template <class Attributes, class Impl>
        class dependent_property<Attributes, Impl, false>
        {
        };
        
        
        template <class T, class D, class Attributes,
            typename getter<T, D>::type Getter>
        class gettable :
            public signaller<T, Attributes>,
            public dependent_property<Attributes, gettable<T, D, Attributes, Getter>>
        {
        public:
            
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using dependent_property_base =
                dependent_property<Attributes, gettable<T, D, Attributes, Getter>>;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            template<class... Properties>
            gettable(D* host, Properties&... properties) :
                dependent_property_base(properties...),
                _host(host)
            {
            }
            
            gettable(const gettable&) = delete;
            
            ~gettable()
            {
            }
            
            auto operator()() const -> result_type
            {
                return (_host->*Getter)();
            }
            
            bool operator == (arg_type other)
            {
                return (*this) == other;
            }
            
            bool operator != (arg_type other)
            {
                return !(*this) == other;
            }
            
        protected:
            
            D*  _host;
            
        };
        
        template <class T, class D, class Attributes,
            typename getter<T, D>::type Getter,
            typename setter<T, D>::type Setter>
        class settable : public gettable<T, D, Attributes, Getter>
        {
        public:
            
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            using gettable<T, D, Attributes, Getter>::gettable;
            
            settable&
            operator += (arg_type rhs)
            {
                return operator=((*this)() + rhs);
            }
            
            settable&
            operator ++ ()
            {
                return operator+=(1);
            }
            
            T
            operator ++ (int)
            {
                T result = (*this)();
                ++(*this);
                return result;
            }
            
            settable&
            operator -= (arg_type rhs)
            {
                return operator=((*this)() - rhs);
            }
            
            settable&
            operator -- ()
            {
                return operator-=(1);
            }
            
            T
            operator -- (int)
            {
                T result = (*this)();
                --(*this);
                return result;
            }
            
            settable&
            operator = (arg_type rhs)
            {
                assign(rhs);
                return *this;
            }
            
            settable&
            operator = (std::nullptr_t)
            {
                assign(nullptr);
                return *this;
            }
            
        private:
            
            void assign(arg_type rhs)
            {
                (gettable<T, D, Attributes, Getter>::_host->*Setter)(rhs);
            }
        };
        
        template <class T, class PropertyType,
                  class Getter, class Setter>
        class dynamic_impl;
        
        template <class T,
            class D,
            class Attributes,
            typename getter<T, D>::type Getter,
            typename setter<T, D>::type Setter>
        class dynamic_impl<T, dynamic<D, Attributes>,
            std::integral_constant<typename getter<T, D>::type, Getter>,
            std::integral_constant<typename setter<T, D>::type, Setter>> :
        public settable<T, D, Attributes, Getter, Setter>
        {
        public:
            
            using settable<T, D, Attributes, Getter, Setter>::settable;
            using settable<T, D, Attributes, Getter, Setter>::operator=;
        };
        
        template <class T,
                  class D,
                  class Attributes,
                  typename getter<T, D>::type Getter>
        class dynamic_impl<T, dynamic<D, Attributes>,
                           std::integral_constant
                            <typename getter<T, D>::type, Getter>,
                           std::integral_constant
                            <typename setter<T, D>::type, nullptr>> :
            public gettable<T, D, Attributes, Getter>
        {
        public:
            using gettable<T, D, Attributes, Getter>::gettable;
        };
        
#if FRESH_REQUIRES_DYNAMIC_PROPERTY_TEMPLATE
        template<class PropertyType>
        struct function_params;
        
        template<class Owner, class Attributes>
        struct function_params<dynamic<Owner, Attributes>>
        {
            template <class T>
            using getter_type = typename getter<T, Owner>::type;
            
            template <class T>
            using setter_type = typename setter<T, Owner>::type;
        };
#endif
    }
}

#endif
