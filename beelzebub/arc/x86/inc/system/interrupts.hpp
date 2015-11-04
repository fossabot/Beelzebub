#pragma once

#include <system/isr.hpp>

namespace Beelzebub { namespace System
{
    /**
     *  Represents the interrupt state of the system
     */
    class Interrupts
    {
    public:
        /*  Statics  */

        static size_t const Count = 256;

        /*  Constructor(s)  */

    protected:
        Interrupts() = default;

    public:
        Interrupts(Interrupts const &) = delete;
        Interrupts & operator =(Interrupts const &) = delete;

        /*  Interrupts  */

        static __bland __forceinline bool AreEnabled()
        {
            size_t flags;

            asm volatile ( "pushf\n\t"
                           "pop %0\n\t"
                           : "=r"(flags) );
            //  Push and pop don't change any flags. Yay!

            return (flags & (size_t)(1 << 9)) != 0;
        }

        static __bland __forceinline void Enable()
        {
            asm volatile ( "sti \n\t" : : : "memory" );
            //  This is a memory barrier to prevent the compiler from moving things around it.
        }

        static __bland __forceinline void Disable()
        {
            asm volatile ( "cli \n\t" : : : "memory" );
            //  This is a memory barrier to prevent the compiler from moving things around it.
        }

        /**
         *  <summary>
         *  Disables interrupts and returns a cookie which allows restoring the
         *  interrupt state as it was before executing this function.
         *  </summary>
         *  <return>
         *  A cookie which allows restoring the interrupt state as it was before
         *  executing this function.
         *  </return>
         */
        static __bland __forceinline int_cookie_t PushDisable()
        {
            int_cookie_t cookie;

            asm volatile ( "pushf      \n\t"
                           "cli        \n\t" // Yes, do it as soon as possible, to avoid interruption.
                           "pop %[dst] \n\t"
                         : [dst]"=r"(cookie)
                         :
                         : "memory");
            
            /*  A bit of wisdom from froggey: ``On second thought, the constraint for flags ["cookie"] in
                interrupt_disable [PushDisableInterrupts] should be "=r", not "=rm". If the compiler decided
                to store [the] flags on the stack and generated an (E|R)SP-relative address, the address would
                end up being off by 4/8 [when passed onto the pop instruction because the stack pointer changed].'' */

            return cookie;
        }

        /**
         *  <summary>
         *  Restores interrupt state based on the given cookie.
         *  </summary>
         *  <return>True if interrupts are now enabled, otherwise false.</return>
         */
        static __bland __forceinline bool RestoreState(const int_cookie_t cookie)
        {
            asm volatile ( "push %[src] \n\t"   //  PUT THE COOKIE DOWN!
                           "popf        \n\t"
                         :
                         : [src]"rm"(cookie)
                         : "memory", "cc" );

            //  Here the cookie can safely be retrieved from the stack because
            //  RSP will change after push, not before.

            return (cookie & (int_cookie_t)(1 << 9)) == 0;
        }
    };

    typedef void (*InterruptEnderFunction)(Beelzebub::System::IsrState * const state);
    typedef void (*InterruptHandlerFunction)(Beelzebub::System::IsrState * const state, InterruptEnderFunction const ender);

    #define ISR_COUNT 256

    /**
     *  The actual IDT.
     */
    __extern uint64_t IsrGates[ISR_COUNT];

    /**
     *  Array of higher-level interrupt handlers.
     */
    __extern InterruptHandlerFunction InterruptHandlers[ISR_COUNT];

    /**
     *  Array of higher-level interrupt handlers.
     */
    __extern InterruptEnderFunction InterruptEnders[ISR_COUNT];
}}
