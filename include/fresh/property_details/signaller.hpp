//
//  signaller.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_signaller_hpp
#define fresh_property_details_signaller_hpp

#include "../signal.hpp"

namespace fresh
{
    template <class SignalType, class ConnectionType>
    struct event_traits;
    
    struct null_signal;
    
    namespace property_details
    {
        template <class EventTraits>
        class signaller_base
        {
        protected:
            
            typename EventTraits::signal_type  _onChanged;
            
        public:
            
            using connection_type = typename EventTraits::connection_type;
            using event_traits = EventTraits;
            using signal_type = typename EventTraits::signal_type;
            
            template <class... Args>
            connection_type
            connect(const std::function<void()>& fn, Args... args)
            {
                return event_traits::connect(_onChanged, fn, args...);
            }
            
            void disconnect_all()
            {
                event_traits::disconnect_all(_onChanged);
            }
        };
        
        class any_class
        {
        };
        
        template <class T, class EventTraits, class F = any_class>
        class signaller : public signaller_base<EventTraits>
        {
            friend F;
            
        public:
            
            using base = signaller_base<EventTraits>;
            
        protected:
            
            void send()
            {
                base::_onChanged();
            }
        };
        
        template <class T, class EventTraits>
        class signaller<T, EventTraits, any_class> : public signaller_base<EventTraits>
        {
        public:
            
            using base = signaller_base<EventTraits>;
            
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
