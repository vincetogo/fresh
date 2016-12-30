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

#include <vector>

namespace fresh
{
    template <class OwnerType, class SignalInfo>
    struct dynamic;
    
    template<class PropertyType>
    struct function_params;
    
    namespace property_details
    {
        template <typename T, typename D>
        struct getter
        {
            using type = T (D::*)() const;
            using result_type = T;
        };
        
        template <typename T, typename D>
        struct getter<T&, D>
        {
            using type = const T& (D::*)() const;
            using result_type = const T&;
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
                
        template <class T, class D, class SignalInfo, typename getter<T, D>::type Getter>
        class gettable : public signaller<T, SignalInfo>
        {
        public:
            
            template<class... Properties>
            gettable(D* host, Properties&... properties) :
                _host(host)
            {
                connect_to_properties(properties...);
            }
            
            gettable(const gettable&) = delete;
            
            auto operator()() const -> typename getter<T,D>::result_type
            {
                return (_host->*Getter)();
            }
            
            bool operator == (typename reference<T>::const_type other)
            {
                return (*this) == other;
            }
            
            bool operator != (typename reference<T>::const_type other)
            {
                return !(*this) == other;
            }
        
        protected:
            
            template <class Property, class... Properties>
            void
            connect_to_properties(Property& first, Properties&... rest)
            {
                auto cnxn = first.connect(
                    [=]()
                    {
                        this->send();
                    }
                    );
                
                _propertyConnections.emplace_back(cnxn);
                
                connect_to_properties(rest...);
            }
            
            void
            connect_to_properties()
            {
            }
            
            D*  _host;
            std::vector<connection_guard> _propertyConnections;
        };
        
        template <class T, class D, class SignalInfo,
                  typename getter<T, D>::type Getter,
                  typename setter<T, D>::type Setter>
        class settable : public gettable<T, D, SignalInfo, Getter>,
            assignable<T, settable<T, D, SignalInfo, Getter, Setter>>
        {
        public:
            
            using assignable_base =
            assignable<T, settable<T, D, SignalInfo, Getter, Setter>>;
            
            using gettable<T, D, SignalInfo, Getter>::gettable;
            using assignable_base::operator=;
            
        private:
            friend assignable_base;
            
            void assign(typename reference<T>::const_type rhs)
            {
                (gettable<T, D, SignalInfo, Getter>::_host->*Setter)(rhs);
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
            public settable<T, D, SignalInfo, Getter, Setter>
        {
        public:
            
            using settable<T, D, SignalInfo, Getter, Setter>::settable;
            using settable<T, D, SignalInfo, Getter, Setter>::operator=;
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
            public gettable<T, D, SignalInfo, Getter>
        {
        public:
            using gettable<T, D, SignalInfo, Getter>::gettable;
        };
    }    
}

#endif
