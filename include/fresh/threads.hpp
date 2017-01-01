//
// threads.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_threads_hpp
#define fresh_threads_hpp

#include <shared_mutex>

#define FRESH_NAMED_LOCK(name, x, lock) lock<decltype(x)> name(x)

#define FRESH_NAMED_LOCK_GUARD(name, x)     FRESH_NAMED_LOCK(name, x, std::lock_guard)
#define FRESH_LOCK_GUARD(x)                 FRESH_NAMED_LOCK_GUARD(fresh_lock_guard, x)

#define FRESH_NAMED_SHARED_GUARD(name, x)   FRESH_NAMED_LOCK(name, x, std::shared_lock)
#define FRESH_SHARED_GUARD(x)               FRESH_NAMED_SHARED_GUARD(fresh_shared_guard, x)

#define FRESH_NAMED_UNIQUE_GUARD(name, x)   FRESH_NAMED_LOCK(name, x, std::unique_lock)
#define FRESH_UNIQUE_GUARD(x)               FRESH_NAMED_UNIQUE_GUARD(fresh_unique_lock, x)


namespace fresh
{
    class atomic_mutex
    {
        
    };
}

namespace std
{
    template <>
    class lock_guard<fresh::atomic_mutex>
    {
    public:
        lock_guard(fresh::atomic_mutex&)
        {
        }
    };
    
    template <>
    class shared_lock<fresh::atomic_mutex>
    {
    public:
        shared_lock(fresh::atomic_mutex&)
        {
        }
    };
    
    template <>
    class unique_lock<fresh::atomic_mutex>
    {
    public:
        unique_lock(fresh::atomic_mutex&)
        {
        }
    };
}

#endif
