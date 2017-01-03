//
// field.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_field_properties_hpp
#define fresh_property_details_field_properties_hpp

#include "assignable.hpp"
#include "format.hpp"
#include "signaller.hpp"
#include "../threads.hpp"

#include <shared_mutex>

namespace fresh
{
    namespace property_details
    {
        template <class T,
            bool = std::is_integral<T>::value && !std::is_same<T, bool>::value>
        struct readable_traits
        {
            using mutex_type = fresh::shared_mutex;
            using value_type = T;
        };
        
        template <class T>
        struct readable_traits<T, true>
        {
            using mutex_type = fresh::atomic_mutex;
            using value_type = std::atomic<T>;
        };
        
        template <class T>
        class readable_field
        {
        public:
            
            readable_field() :
                _value()
            {
            }
            
            readable_field(typename format<T>::arg_type value) :
                _value(value)
            {
            }
            
            readable_field(const readable_field& other) :
                _value(other())
            {
            }
            
            readable_field(std::nullptr_t) :
                _value(nullptr)
            {
            }
            
            typename format<T>::result_type operator () () const
            {
                FRESH_READ_GUARD(_mutex);
                
                return _value;
            }
            
            bool operator == (typename format<T>::arg_type other) const
            {
                FRESH_READ_GUARD(_mutex);

                return _value == other;
            }
            
            bool operator != (typename format<T>::arg_type other) const
            {
                return !operator==(other);
            }
            
        protected:
            
            mutable typename readable_traits<T>::mutex_type _mutex;
            typename readable_traits<T>::value_type         _value;
        };
                
        template <class T,
                  class Impl>
        class writable_field_base :
            public readable_field<T>
        {
        public:
            using base = readable_field<T>;
            
            using base::base;
                        
            void
            assign(typename format<T>::arg_type rhs)
            {
                {
                    FRESH_WRITE_GUARD(base::_mutex);
                    readable_field<T>::_value = rhs;
                }
                
                ((Impl*)this)->on_assign();
            }
        };
        
        template <class T,
                  class EventTraits,
                  class SignalFriend>
        class writable_field :
            public writable_field_base<T,
                writable_field<T, EventTraits, SignalFriend>>,
            public assignable<typename readable_traits<T>::value_type,
                writable_field<T, EventTraits, SignalFriend>>,
            public signaller<T, EventTraits, SignalFriend>
        {
        public:
            using base = writable_field_base<T,
                writable_field<T, EventTraits, SignalFriend>>;
            using assignable_base = assignable<typename readable_traits<T>::value_type,
                writable_field<T, EventTraits, SignalFriend>>;
            
            writable_field() :
                base()
            {
            }
            
            writable_field(typename format<T>::arg_type other) :
                base(other)
            {
            }
            
            writable_field(std::nullptr_t) :
                base(nullptr)
            {
            }
            
            writable_field(const writable_field& other) :
                base(other())
            {
            }
            
            using assignable_base::operator=;
            
        private:
            friend base;
            friend assignable_base;
            
            void
            on_assign()
            {
                signaller<T, EventTraits, SignalFriend>::send();
            }
        };
        
        template <class T,
                  class SignalFriend>
        class writable_field<T, null_signal, SignalFriend> :
            public writable_field_base<T,
                writable_field<T, null_signal, SignalFriend>>,
            public assignable<typename readable_traits<T>::value_type,
                writable_field<T, null_signal, SignalFriend>>
        {
        public:
            using base = writable_field_base<T,
                writable_field<T, null_signal, SignalFriend>>;
            using assignable_base = assignable<typename readable_traits<T>::value_type,
                writable_field<T, null_signal, SignalFriend>>;
            
            using base::base;
            using assignable_base::operator=;
            
        private:
            friend base;
            
            void
            on_assign()
            {
            }
        };
    }
}

#endif
