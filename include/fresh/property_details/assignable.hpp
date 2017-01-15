//
//  assignable.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_property_details_assignable_hpp
#define fresh_property_details_assignable_hpp

#include "traits.hpp"

#include <cstddef>

namespace fresh
{
    namespace property_details
    {
        template <class T, class Attributes, class Impl>
        class assignable
        {
        public:
            
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            result_type operator () () const
            {
                read_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
                return ((Impl*)this)->_value;
            }
            
            Impl&
            operator += (arg_type rhs)
            {
                write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
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
                write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
                T result = (*(Impl*)this)();
                ++(*this);
                return result;
            }
            
            Impl&
            operator -= (arg_type rhs)
            {
                write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
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
                write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
                T result = (*(Impl*)this)();
                --(*this);
                return result;
            }
            
        protected:
            
            Impl&
            operator = (arg_type rhs)
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
            
            void
            assign(arg_type rhs)
            {
                {
                    write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                    ((Impl*)this)->_value = rhs;
                }
                
                ((Impl*)this)->on_assign();
            }
        };
        
        template <class T, class Attributes, class Impl>
        class assignable<std::atomic<T>, Attributes, Impl>
        {
        public:
            
            T operator () () const
            {
                return ((Impl*)this)->_value;
            }
            
            Impl&
            operator += (T rhs)
            {
                ((Impl*)this)->_value.fetch_add(rhs);
                ((Impl*)this)->on_assign();
                
                return *((Impl*)this);
            }
            
            Impl&
            operator ++ ()
            {
                return operator+=(1);
            }
            
            T
            operator ++ (int)
            {
                auto result = ((Impl*)this)->_value.fetch_add(1);
                
                ((Impl*)this)->on_assign();
                
                return result;
            }
            
            Impl&
            operator -= (T rhs)
            {
                ((Impl*)this)->_value.fetch_add(-rhs);
                ((Impl*)this)->on_assign();
                
                return *((Impl*)this);
            }
            
            Impl&
            operator -- ()
            {
                return operator-=(1);
            }
            
            T
            operator -- (int)
            {
                auto result = ((Impl*)this)->_value.fetch_add(-1);
                
                ((Impl*)this)->on_assign();
                
                return result;
            }
            
        protected:
            
            Impl&
            operator = (T rhs)
            {
                ((Impl*)this)->_value = rhs;
                ((Impl*)this)->on_assign();
                
                return *(Impl*)this;
            }
            
            Impl&
            operator = (std::nullptr_t)
            {
                ((Impl*)this)->_value = nullptr;
                ((Impl*)this)->on_assign();
                
                return *(Impl*)this;
            }
        };
    }
}

#endif /* assignable_h */
