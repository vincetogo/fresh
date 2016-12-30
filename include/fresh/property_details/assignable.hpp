//
//  assignable.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_assignable_hpp
#define fresh_property_details_assignable_hpp

#include <cstddef>

namespace fresh
{
    namespace property_details
    {
        template <typename T>
        struct reference
        {
            using type = T&;
            using const_type = const T&;
        };
        
        template <typename T>
        struct reference<T&>
        {
            using type = T&;
            using const_type = const T&;
        };

        template <class T, class Impl>
        class assignable
        {
        public:
            
            Impl&
            operator += (typename reference<T>::const_type rhs)
            {
                return operator=((*(Impl*)this)() + rhs);
            }
            
            Impl&
            operator ++ ()
            {
                return operator+=(1);
            }
            
            T
            operator ++ (int)
            {
                T result = (*(Impl*)this)();
                ++(*this);
                return result;
            }
            
            Impl&
            operator -= (typename reference<T>::const_type rhs)
            {
                return operator=((*(Impl*)this)() - rhs);
            }
            
            Impl&
            operator -- ()
            {
                return operator-=(1);
            }
            
            T
            operator -- (int)
            {
                T result = (*(Impl*)this)();
                --(*this);
                return result;
            }
            
        protected:
            
            Impl&
            operator = (typename reference<T>::const_type rhs)
            {
                ((Impl*)this)->assign(rhs);
                return *(Impl*)this;
            }
            
            Impl&
            operator = (std::nullptr_t)
            {
                ((Impl*)this)->assign(nullptr);
                return *(Impl*)this;
            }
        };
    }
}

#endif /* assignable_h */
