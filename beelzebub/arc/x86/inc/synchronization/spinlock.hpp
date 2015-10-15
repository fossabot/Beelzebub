#pragma once

#include <metaprogramming.h>

namespace Beelzebub { namespace Synchronization
{
    typedef size_t volatile spinlock_t;

    //  Lemme clarify here.
    //  For non-SMP builds, SMP spinlocks are gonna be dummies.
    //  For SMP builds, all spinlocks are implemented.

#if   defined(__BEELZEBUB_SETTINGS_NO_SMP)
    /**
     *  Busy-waiting re-entrant synchronization primitive.
     */
    template<bool SMP = true>
    struct Spinlock { };

    /**
     *  Busy-waiting re-entrant synchronization primitive.
     */
    template<>
    struct Spinlock<false>
#else
    /**
     *  Busy-waiting re-entrant synchronization primitive.
     */
    template<bool SMP = true>
    struct Spinlock
#endif
    {
    public:

        /*  Constructor(s)  */

        Spinlock() = default;
        Spinlock(Spinlock const &) = delete;
        Spinlock & operator =(Spinlock const &) = delete;

        /*  Destructor  */

#ifdef __BEELZEBUB__DEBUG
        __bland ~Spinlock();
#endif

        /*  Operations  */

        /**
         *  Acquire the spinlock, if possible.
         */
        __bland __forceinline __must_check bool TryAcquire() volatile
        {
            spinlock_t oldValue = __sync_lock_test_and_set(&this->Value, 1);

            return !oldValue;
        }

        /**
         *  Awaits for the spinlock to be freed.
         *  Does not acquire the lock.
         */
        __bland __forceinline void Spin() const volatile
        {
            do
            {
                asm volatile ("pause");
            } while (this->Value);
        }

        /**
         *  Checks if the spinlock is free. If not, it awaits.
         *  Does not acquire the lock.
         */
        __bland __forceinline void Await() const volatile
        {
            while (this->Value)
            {
                asm volatile ("pause");
            }
        }

        /**
         *  Acquire the spinlock, waiting if necessary.
         */
        __bland __forceinline void Acquire() volatile
        {
            while (__sync_lock_test_and_set(&this->Value, 1))
                this->Spin();
        }

        /**
         *  Acquire the spinlock, waiting if necessary.
         *  Includes a pointer in the memory barrier, if supported.
         */
        __bland __forceinline void Acquire(void * const ptr) volatile
        {
            while (__sync_lock_test_and_set(&this->Value, 1, ptr))
                this->Spin();
        }

        /**
         *  Release the spinlock.
         */
        __bland __forceinline void Release() volatile
        {
            __sync_lock_release(&this->Value);
        }

        /**
         *  Release the spinlock.
         *  Includes a pointer in the memory barrier.
         */
        __bland __forceinline void Release(void * const ptr) volatile
        {
            __sync_lock_release(&this->Value, ptr);
        }

        /**
         *  Checks whether the spinlock is free or not.
         */
        __bland __forceinline __must_check bool Check() const volatile
        {
            return this->Value == 0;
        }

        /*  Properties  */

        __bland __forceinline spinlock_t GetValue() const volatile
        {
            return this->Value;
        }

        /*  Fields  */

    private:

        spinlock_t Value; 
    };

#if   defined(__BEELZEBUB_SETTINGS_NO_SMP)
    /**
     *  Busy-waiting re-entrant synchronization primitive.
     */
    template<>
    struct Spinlock<true>
    {
    public:

        /*  Constructor(s)  */

        Spinlock() = default;
        Spinlock(Spinlock const &) = delete;
        Spinlock & operator =(Spinlock const &) = delete;

        /*  Operations  */

        /**
         *  Acquire the spinlock, if possible.
         */
        __bland __forceinline __must_check constexpr bool TryAcquire() const volatile
        {
            return true;
        }

        /**
         *  Awaits for the spinlock to be freed.
         *  Does not acquire the lock.
         */
        __bland __forceinline void Spin() const volatile
        {
            //  Do nothing.
        }

        /**
         *  Checks if the spinlock is free. If not, it awaits.
         *  Does not acquire the lock.
         */
        __bland __forceinline void Await() const volatile
        {
            //  Do nothing.
        }

        /**
         *  Acquire the spinlock, waiting if necessary.
         */
        __bland __forceinline void Acquire() const volatile
        {
            //  Do nothing.
        }

        /**
         *  Acquire the spinlock, waiting if necessary.
         *  Includes a pointer in the memory barrier, if supported.
         */
        __bland __forceinline void Acquire(void * const ptr) const volatile
        {
            //  Do nothing.
        }

        /**
         *  Release the spinlock.
         */
        __bland __forceinline void Release() const volatile
        {
            //  Do nothing.
        }

        /**
         *  Release the spinlock.
         *  Includes a pointer in the memory barrier.
         */
        __bland __forceinline void Release(void * const ptr) const volatile
        {
            //  Do nothing.
        }

        /**
         *  Checks whether the spinlock is free or not.
         */
        __bland __forceinline __must_check constexpr bool Check() const volatile
        {
            return true;
        }

        /*  Properties  */

        __bland __forceinline constexpr spinlock_t GetValue() const volatile
        {
            return (spinlock_t)0;
        }
    };
#endif
}}

//  Very sad note: GCC doesn't support protecting additional pointers in
//  the memory barrier. Nevertheless, I have added the feature.
