//
//  signaller.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_signaller_hpp
#define fresh_property_details_signaller_hpp

#include "../event.hpp"

namespace fresh
{
    struct null_signal;
    
    namespace property_details
    {
        template <class PropertyTraits>
        class signaller_base
        {
        protected:
            
            typename PropertyTraits::signal_type  _onChanged;
            
        public:
            
            using connection_type = typename PropertyTraits::connection_type;
            using event_traits = PropertyTraits;
            using signal_type = typename PropertyTraits::signal_type;
            
            template <class... Args>
            connection_type
            connect(const std::function<void()>& fn, Args... args)
            {
                return event_traits::connect(_onChanged, fn, args...);
            }
        };
        
        class any_class
        {
        };
        
        template <class T, class PropertyTraits, class F = any_class>
        class signaller : public signaller_base<PropertyTraits>
        {
            friend F;
            
        public:
            
            using base = signaller_base<PropertyTraits>;
            
        protected:
            
            void send()
            {
                base::_onChanged();
            }
        };
        
        template <class T, class PropertyTraits>
        class signaller<T, PropertyTraits, any_class> : public signaller_base<PropertyTraits>
        {
        public:
            
            using base = signaller_base<PropertyTraits>;
            
            void send()
            {
                base::_onChanged();
            }
        };
        
        template <class T, class F>
        class signaller<T, null_signal, F>
        {
        };
        
        template <class T>
        class signaller<T, null_signal, any_class>
        {
        };
    }
}

#endif /* signaller_h */
