//
// property.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_dynamic_properties_hpp
#define fresh_property_details_dynamic_properties_hpp

#include "format.hpp"
#include "signaller.hpp"

#include <vector>

namespace fresh
{
    template <class OwnerType, class PropertyTraits>
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
        
        
        template <class PropertyTraits>
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
                        this->send();
                    });
                
                _propertyConnections.push_back(std::move(cnxn));
                
                connect_to_properties(rest...);
            }
            
            void
            connect_to_properties()
            {
            }
            
            std::vector<typename PropertyTraits::connection_type>
                _propertyConnections;
        };
        
        template <>
        class dependent_property<null_signal>
        {
        };
        
        
        
        template <class T, class D, class PropertyTraits,
            typename getter<T, D>::type Getter>
        class gettable : public signaller<T, PropertyTraits>,
            public dependent_property<PropertyTraits>
        {
        public:
            
            template<class... Properties>
            gettable(D* host, Properties&... properties) :
                _host(host)
            {
            }
            
            gettable(const gettable&) = delete;
            
            ~gettable()
            {
            }
            
            auto operator()() const -> typename format<T>::result_type
            {
                return (_host->*Getter)();
            }
            
            bool operator == (typename format<T>::arg_type other)
            {
                return (*this) == other;
            }
            
            bool operator != (typename format<T>::arg_type other)
            {
                return !(*this) == other;
            }
            
        protected:
            
            D*  _host;
            
        };
        
        template <class T, class D, class PropertyTraits,
            typename getter<T, D>::type Getter,
            typename setter<T, D>::type Setter>
        class settable : public gettable<T, D, PropertyTraits, Getter>
        {
        public:
            
            using gettable<T, D, PropertyTraits, Getter>::gettable;
            
            settable&
            operator += (typename format<T>::arg_type rhs)
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
            operator -= (typename format<T>::arg_type rhs)
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
            operator = (typename format<T>::arg_type rhs)
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
            
            void assign(typename format<T>::arg_type rhs)
            {
                (gettable<T, D, PropertyTraits, Getter>::_host->*Setter)(rhs);
            }
        };
        
        template <class T, class PropertyType,
                  class Getter, class Setter>
        class dynamic_impl;
        
        template <class T,
            class D,
            class PropertyTraits,
            typename getter<T, D>::type Getter,
            typename setter<T, D>::type Setter>
        class dynamic_impl<T, dynamic<D, PropertyTraits>,
            std::integral_constant<typename getter<T, D>::type, Getter>,
            std::integral_constant<typename setter<T, D>::type, Setter>> :
        public settable<T, D, PropertyTraits, Getter, Setter>
        {
        public:
            
            using settable<T, D, PropertyTraits, Getter, Setter>::settable;
            using settable<T, D, PropertyTraits, Getter, Setter>::operator=;
        };
        
        template <class T,
                  class D,
                  class PropertyTraits,
                  typename getter<T, D>::type Getter>
        class dynamic_impl<T, dynamic<D, PropertyTraits>,
                           std::integral_constant
                            <typename getter<T, D>::type, Getter>,
                           std::integral_constant
                            <typename setter<T, D>::type, nullptr>> :
            public gettable<T, D, PropertyTraits, Getter>
        {
        public:
            using gettable<T, D, PropertyTraits, Getter>::gettable;
        };
    }    
}

#endif
