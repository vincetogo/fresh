//
// property.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_dynamic_properties_hpp
#define fresh_property_details_dynamic_properties_hpp

#include "assignable.hpp"
#include "signaller.hpp"

namespace fresh
{
    template <class OwnerType, class SignalInfo>
    struct dynamic;
    
    namespace property_details
    {
        template <typename T, typename D>
        struct getter
        {
            using type = T& (D::*)();
        };
        
        template <typename T, typename D>
        struct getter<const T, D>
        {
            using type = const T& (D::*)();
        };
        
        template <typename T, typename D>
        struct setter
        {
            using type = void (D::*)(const T&);
        };
        
        template <typename T, typename D>
        struct setter<const T, D>
        {
            using type = void (D::*)(const T&);
        };
        
        template <class T, class D, typename getter<T, D>::type Getter>
        class gettable
        {
        public:
            
            gettable(D* host) :
                _host(host)
            {
                
            }
            
            T& operator()()
            {
                return (_host->*Getter)();
            }
            
            const T& operator()() const
            {
                return (_host->*Getter)();
            }
            
            bool operator == (const T& other)
            {
                return (*this) == other;
            }
            
            bool operator != (const T& other)
            {
                return !(*this) == other;
            }
        
        protected:
            D*  _host;
            
        private:
            gettable(const gettable&);
        };
        
        template <class T, class D,
                  typename getter<T, D>::type Getter,
                  typename setter<T, D>::type Setter>
        class settable : public gettable<T, D, Getter>,
            assignable<T, settable<T, D, Getter, Setter>>
        {
        public:
            
            using assignable_base =
            assignable<T, settable<T, D, Getter, Setter>>;
            
            using gettable<T, D, Getter>::gettable;
            using assignable_base::operator=;
            
        private:
            friend assignable_base;
            
            void assign(const T& rhs)
            {
                (gettable<T, D, Getter>::_host->*Setter)(rhs);
            }
        };
        
        template <class T, class PropertyType,
                  class Getter, class Setter>
        class dynamic_impl;
        
        template <class T,
                  class D,
                  class SignalInfo,
                  typename getter<T, D>::type Getter,
                  typename setter<T, D>::type Setter>
        class dynamic_impl<T, dynamic<D, SignalInfo>,
                           std::integral_constant
                            <typename getter<T, D>::type, Getter>,
                           std::integral_constant
                            <typename setter<T, D>::type, Setter>> :
            public settable<T, D, Getter, Setter>,
            public signaller<T, SignalInfo, D>
        {
        public:
            
            using settable<T, D, Getter, Setter>::settable;
            using settable<T, D, Getter, Setter>::operator=;
        };
        
        template <class T,
                  class D,
                  class SignalInfo,
                  typename getter<T, D>::type Getter>
        class dynamic_impl<T, dynamic<D, SignalInfo>,
                           std::integral_constant
                            <typename getter<T, D>::type, Getter>,
                           std::integral_constant
                            <typename setter<T, D>::type, nullptr>> :
        public gettable<T, D, Getter>
        {
        public:
            using gettable<T, D, Getter>::gettable;
        };
    }    
}

#endif
