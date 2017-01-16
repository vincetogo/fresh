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
        class assignable;
        
        template <class T, class Assignable, bool = has_add<T>::value>
        class assignable_add
        {
        };
        
        template <class T, class Attributes, class Impl>
        class assignable_add<T, assignable<T, Attributes, Impl>, true>
        {
            using Assignable = assignable<T, Attributes, Impl>;
            
        public:
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            Impl&
            operator+= (arg_type rhs)
            {
                {
                    write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                    
                    ((Impl*)this)->_value = ((Impl*)this)->_value + rhs;
                }
                
                ((Impl*)this)->on_assign();
                
                return *(Impl*)this;
            }
            
            Impl&
            operator++ ()
            {
                return operator+=(1);
            }
            
            T
            operator++ (int)
            {
                write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
                T result = (*(Impl*)this)();
                ++(*this);
                return result;
            }
        };
        
        template <class T, class Attributes, class Impl>
        class assignable_add<std::atomic<T>, assignable<std::atomic<T>, Attributes, Impl>, true>
        {
        public:
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            Impl&
            operator+= (T rhs)
            {
                ((Impl*)this)->_value.fetch_add(rhs);
                ((Impl*)this)->on_assign();
                
                return *((Impl*)this);
            }
            
            Impl&
            operator++ ()
            {
                return operator+=(1);
            }
            
            T
            operator++ (int)
            {
                auto result = ((Impl*)this)->_value.fetch_add(1);
                ((Impl*)this)->on_assign();
                
                return result;
            }
        };
        
        template <class T, class Assignable, bool = has_subtract<T>::value>
        class assignable_subtract
        {
        };
        
        template <class T, class Attributes, class Impl>
        class assignable_subtract<T, assignable<T, Attributes, Impl>, true>
        {
            using Assignable = assignable<T, Attributes, Impl>;
            
        public:
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            Impl&
            operator-= (arg_type rhs)
            {
                {
                    write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                    
                    ((Impl*)this)->_value = ((Impl*)this)->_value - rhs;
                }
                
                ((Impl*)this)->on_assign();
                
                return *(Impl*)this;
            }
            
            Impl&
            operator-- ()
            {
                return operator-=(1);
            }
            
            T
            operator-- (int)
            {
                write_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
                T result = (*(Impl*)this)();
                --(*this);
                return result;
            }
        };
        
        template <class T, class Attributes, class Impl>
        class assignable_subtract<std::atomic<T>, assignable<std::atomic<T>, Attributes, Impl>, true>
        {
        public:
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            Impl&
            operator-= (T rhs)
            {
                ((Impl*)this)->_value.fetch_add(-rhs);
                ((Impl*)this)->on_assign();
                
                return *((Impl*)this);
            }
            
            Impl&
            operator-- ()
            {
                return operator-=(1);
            }
            
            T
            operator-- (int)
            {
                auto result = ((Impl*)this)->_value.fetch_add(-1);
                ((Impl*)this)->on_assign();
                
                return result;
            }
        };
        
        template <class T, class Attributes, class Impl>
        class assignable :
            public assignable_add<T, assignable<T, Attributes, Impl>>,
            public assignable_subtract<T, assignable<T, Attributes, Impl>>
        {
        public:
            
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            result_type operator() () const
            {
                read_lock<typename Impl::mutex_type> lock(((Impl*)this)->_mutex);
                
                return ((Impl*)this)->_value;
            }
            
        protected:
            
            friend assignable_add<T, assignable<T, Attributes, Impl>>;
            friend assignable_subtract<T, assignable<T, Attributes, Impl>>;
            
            Impl&
            operator= (arg_type rhs)
            {
                assign(rhs);
                return *(Impl*)this;
            }
            
            Impl&
            operator= (std::nullptr_t)
            {
                assign(nullptr);
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
        class assignable<std::atomic<T>, Attributes, Impl> :
            public assignable_add<std::atomic<T>,
                assignable<std::atomic<T>, Attributes, Impl>>,
            public assignable_subtract<std::atomic<T>,
                assignable<std::atomic<T>, Attributes, Impl>>
        {
        public:
            
            using arg_type = typename property_traits<T, Attributes>::arg_type;
            using result_type = typename property_traits<T, Attributes>::result_type;
            
            T operator() () const
            {
                return ((Impl*)this)->_value;
            }
            
        protected:
            
            friend assignable_add<std::atomic<T>,
                assignable<std::atomic<T>, Attributes, Impl>>;
            friend assignable_subtract<std::atomic<T>,
                assignable<std::atomic<T>, Attributes, Impl>>;
            
            Impl&
            operator= (T rhs)
            {
                ((Impl*)this)->_value = rhs;
                ((Impl*)this)->on_assign();
                
                return *(Impl*)this;
            }
            
            Impl&
            operator= (std::nullptr_t)
            {
                ((Impl*)this)->_value = nullptr;
                ((Impl*)this)->on_assign();
                
                return *(Impl*)this;
            }
        };
    }
}

#endif /* assignable_h */
