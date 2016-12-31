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
    struct signal_info;
    
    struct null_signal;
    
    namespace property_details
    {
        template <class SignalInfo>
        class signaller_base
        {
        protected:
            
            typename SignalInfo::signal_type  _onChanged;
            
        public:
            
            using connection_type = typename SignalInfo::connection_type;
            using signal_info = SignalInfo;
            using signal_type = typename SignalInfo::signal_type;
            
            template <class... Args>
            connection_type
            connect(const std::function<void()>& fn, Args... args)
            {
                return signal_info::connect(_onChanged, fn, args...);
            }
            
            void disconnect_all()
            {
                signal_info::disconnect_all(_onChanged);
            }
        };
        
        class any_class
        {
        };
        
        template <class T, class SignalInfo, class F = any_class>
        class signaller : public signaller_base<SignalInfo>
        {
            friend F;
            
        public:
            
            using base = signaller_base<SignalInfo>;
            
        protected:
            
            void send()
            {
                base::_onChanged();
            }
        };
        
        template <class T, class SignalInfo>
        class signaller<T, SignalInfo, any_class> : public signaller_base<SignalInfo>
        {
        public:
            
            using base = signaller_base<SignalInfo>;
            
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
