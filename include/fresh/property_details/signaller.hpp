//
//  signaller.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_signaller_hpp
#define fresh_property_details_signaller_hpp

#include "traits.hpp"

#include "../event.hpp"

namespace fresh
{
    struct null_signal;
    
    namespace property_details
    {
        template <class Attributes>
        class signaller_base
        {
        protected:
            
            typename Attributes::event_type  _onChanged;
            
        public:
            
            using connection_type = typename Attributes::connection_type;
            using attributes = Attributes;
            using event_type = typename Attributes::event_type;
            
            template <class... Args>
            connection_type
            connect(const std::function<void()>& fn, Args... args)
            {
                return attributes::connect(_onChanged, fn, args...);
            }
        };
                
        template <class T,
            class Attributes,
            class F = void,
            bool = has_event<Attributes>::value>
        class signaller : public signaller_base<Attributes>
        {
            friend F;
            
        public:
            
            using base = signaller_base<Attributes>;
            
        protected:
            
            void send()
            {
                base::_onChanged();
            }
        };
        
        template <class T, class Attributes>
        class signaller<T, Attributes, void, true> : public signaller_base<Attributes>
        {
        public:
            
            using base = signaller_base<Attributes>;
            
            void send()
            {
                base::_onChanged();
            }
        };
        
        template <class T, class Attributes, class F>
        class signaller<T, Attributes, F, false>
        {
        };
        
        template <class T, class Attributes>
        class signaller<T, Attributes, void, false>
        {
        };
    }
}

#endif /* signaller_h */
