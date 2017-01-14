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
#include "traits.hpp"
#include "../threads.hpp"

#include <shared_mutex>

namespace fresh
{
    template <class Attributes>
    struct has_event;
    
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
                
        template <class T,
                  class Attributes,
                  class Impl>
        class writable_field_base
        {
        public:
            
            using mutex_type = typename readable_traits<T>::mutex_type;
            
            writable_field_base() :
                _value()
            {
            }
            
            writable_field_base(typename format<T, Attributes>::arg_type value) :
                _value(value)
            {
            }
            
            writable_field_base(const writable_field_base& other) :
                _value(other())
            {
            }
            
            writable_field_base(std::nullptr_t) :
                _value(nullptr)
            {
            }
            
            typename format<T, Attributes>::result_type operator () () const
            {
                read_lock<mutex_type> lock(_mutex);
                
                return _value;
            }
            
            bool operator == (T other) const
            {
                return operator()() == other;
            }
            
            bool operator != (T other) const
            {
                return !operator==(other);
            }
            
            void
            assign(typename format<T, Attributes>::arg_type rhs)
            {
                {
                    write_lock<mutex_type> lock(_mutex);
                    _value = rhs;
                }
                
                ((Impl*)this)->on_assign();
            }
            
        protected:
            
            mutable typename readable_traits<T>::mutex_type _mutex;
            typename readable_traits<T>::value_type         _value;
        };
        
        template <class T,
                  class Attributes,
                  class SignalFriend,
                  bool = has_event<Attributes>::value>
        class writable_field :
            public writable_field_base<T, Attributes,
                writable_field<T, Attributes, SignalFriend>>,
            public assignable<typename readable_traits<T>::value_type,
                Attributes,
                writable_field<T, Attributes, SignalFriend>>,
            public signaller<T, Attributes, SignalFriend>
        {
        public:
            using base = writable_field_base<T, Attributes,
                writable_field<T, Attributes, SignalFriend>>;
            using assignable_base =
                assignable<typename readable_traits<T>::value_type,
                    Attributes,
                    writable_field<T, Attributes, SignalFriend>>;
            
            writable_field() :
                base()
            {
            }
            
            writable_field(typename format<T, Attributes>::arg_type other) :
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
                signaller<T, Attributes, SignalFriend>::send();
            }
        };
        
        template <class T,
                  class Attributes,
                  class SignalFriend>
        class writable_field<T, Attributes, SignalFriend, false> :
            public writable_field_base<T, Attributes,
                writable_field<T, Attributes, SignalFriend>>,
            public assignable<typename readable_traits<T>::value_type,
                Attributes,
                writable_field<T, Attributes, SignalFriend>>
        {
        public:
            using base = writable_field_base<T, Attributes,
                writable_field<T, Attributes, SignalFriend>>;
            using assignable_base =
                assignable<typename readable_traits<T>::value_type,
                    Attributes,
                    writable_field<T, Attributes, SignalFriend>>;
            
            using base::base;
            using assignable_base::operator=;
            
        private:
            friend base;
            friend assignable_base;
            
            void
            on_assign()
            {
            }
        };
    }
}

#endif
