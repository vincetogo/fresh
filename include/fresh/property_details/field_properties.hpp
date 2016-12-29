//
// field.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_field_properties_hpp
#define fresh_property_details_field_properties_hpp

#include "assignable.hpp"
#include "signaller.hpp"

namespace fresh
{
    namespace property_details
    {
        template <class T>
        class readable_field
        {
        public:
            
            readable_field() :
                _value()
            {
            }
            
            readable_field(const T& value) :
                _value(value)
            {
            }
            
            readable_field(std::nullptr_t) :
                _value(nullptr)
            {
            }
            
            const T& operator () () const
            {
                return _value;
            }
            
            bool operator == (const T& other) const
            {
                return _value == other;
            }
            
            bool operator != (const T& other) const
            {
                return !operator==(other);
            }
            
        protected:
            
            T  _value;
        };
        
        template <typename T>
        using assignment_test_t = bool (*)(const T& currValue, const T& newValue);
        
        template <class T,
                  assignment_test_t<T> AssignmentTest,
                  class Impl>
        class writable_field_base :
            public readable_field<T>,
            public assignable<T, writable_field_base<T, AssignmentTest, Impl>>
        {
        public:
            using base = readable_field<T>;
            
            using base::base;
            using assignable<T, writable_field_base<T, AssignmentTest, Impl>>::operator=;
            
        private:
            friend assignable<T, writable_field_base<T, AssignmentTest, Impl>>;
            
            void
            assign(const T& rhs)
            {
                if (AssignmentTest(readable_field<T>::_value, rhs))
                {
                    readable_field<T>::_value = rhs;
                    ((Impl*)this)->on_assign();
                }
            }
        };
        
        template <class T,
                  assignment_test_t<T> AssignmentTest,
                  class SignalInfo,
                  class SignalFriend>
        class writable_field :
            public writable_field_base<T, AssignmentTest,
                writable_field<T, AssignmentTest, SignalInfo, SignalFriend>>,
            public signaller<T, SignalInfo, SignalFriend>
        {
        public:
            using base = writable_field_base<T, AssignmentTest,
                writable_field<T, AssignmentTest, SignalInfo, SignalFriend>>;
            
            writable_field() :
                base()
            {
            }
            
            writable_field(const T& other) :
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
            
            using base::operator=;
            
        private:
            friend base;
            
            void
            on_assign()
            {
                signaller<T, SignalInfo, SignalFriend>::send();
            }
        };
        
        template <class T,
                  assignment_test_t<T> AssignmentTest,
                  class SignalFriend>
        class writable_field<T, AssignmentTest, null_signal, SignalFriend> :
            public writable_field_base<T, AssignmentTest,
                writable_field<T, AssignmentTest, null_signal, SignalFriend>>
        {
        public:
            using base = writable_field_base<T, AssignmentTest,
                writable_field<T, AssignmentTest, null_signal, SignalFriend>>;
            
            using base::base;
            using base::operator=;
            
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
