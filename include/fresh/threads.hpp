//
// threads.hpp
//
//  Created by Vincent Tourangeau on 12/29/16.
//  Copyright © 2016 Vincent Tourangeau. All rights reserved.
//

#ifndef fresh_threads_hpp
#define fresh_threads_hpp

#include <mutex>

#ifdef __APPLE__

#include <AvailabilityMacros.h>

    #if MACOSX_DEPLOYMENT_TARGET > MACOS_X_VERSION_10_11

#define FRESH_USE_STD_SHARED_MUTEX 1

    #else
        #define FRESH_USE_STD_SHARED_MUTEX 0
    #endif

#else
    #define FRESH_USE_STD_SHARED_MUTEX 1
#endif

#include <shared_mutex>


namespace fresh
{
    class atomic_mutex
    {
        
    };

#if FRESH_USE_STD_SHARED_MUTEX
    using shared_mutex = std::shared_timed_mutex;
    
    template <class T>
    using shared_lock = std::shared_lock<T>;

#else
    using shared_mutex = std::mutex;
    
    template <class T>
    using shared_lock = std::lock_guard<T>;

#endif

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

#define FRESH_NAMED_LOCK(name, x, lock) lock<decltype(x)> name(x)

#define FRESH_NAMED_LOCK_GUARD(name, x)     FRESH_NAMED_LOCK(name, x, std::lock_guard)
#define FRESH_LOCK_GUARD(x)                 FRESH_NAMED_LOCK_GUARD(fresh_lock_guard, x)

#define FRESH_NAMED_SHARED_GUARD(name, x)   FRESH_NAMED_LOCK(name, x, fresh::shared_lock)
#define FRESH_SHARED_GUARD(x)               FRESH_NAMED_SHARED_GUARD(fresh_shared_guard, x)

#define FRESH_NAMED_UNIQUE_GUARD(name, x)   FRESH_NAMED_LOCK(name, x, std::unique_lock)
#define FRESH_UNIQUE_GUARD(x)               FRESH_NAMED_UNIQUE_GUARD(fresh_unique_lock, x)

#endif
