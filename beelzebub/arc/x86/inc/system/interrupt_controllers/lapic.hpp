/*
    Copyright (c) 2015 Alexandru-Mihai Maftei. All rights reserved.


    Developed by: Alexandru-Mihai Maftei
    aka Vercas
    http://vercas.com | https://github.com/vercas/Beelzebub

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal with the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimers.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimers in the
        documentation and/or other materials provided with the distribution.
      * Neither the names of Alexandru-Mihai Maftei, Vercas, nor the names of
        its contributors may be used to endorse or promote products derived from
        this Software without specific prior written permission.


    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    WITH THE SOFTWARE.

    ---

    You may also find the text of this license in "LICENSE.md", along with a more
    thorough explanation regarding other files.
*/

#pragma once

#include "irqs.hpp"
#include <utils/bitfields.hpp>
#include <beel/handles.h>

#define LAPICREGFUNC1(name, prettyName, type)                             \
static __forceinline type MCATS2(Get, prettyName)()               \
{                                                                         \
    return type(ReadRegister(LapicRegister::name));                       \
}                                                                         \
static __forceinline void MCATS2(Set, prettyName)(const type val) \
{                                                                         \
    WriteRegister(LapicRegister::name, val.Value);                        \
}

namespace Beelzebub { namespace System { namespace InterruptControllers
{
    class Lapic;

    /**
     *  <summary>Known LAPIC register offsets.</summary>
     */
    enum class LapicRegister : uint16_t
    {
        LapicId                      = 0x0002,
        SpuriousInterruptVector      = 0x000F,
        EndOfInterrupt               = 0x000B,
        InterruptCommandRegisterLow  = 0x0030,
        InterruptCommandRegisterHigh = 0x0031,
        TimerLvt                     = 0x0032,
        TimerInitialCount            = 0x0038,
        TimerCurrentCount            = 0x0039,
        TimerDivisor                 = 0x003E,
    };

    /**
     *  <summary>
     *  Represents the contents of the Spurious-Interrupt Vector Register of the
     *  LAPIC.
     *  </summary>
     */
    struct LapicSvr
    {
        friend class Lapic;

        /*  Bit structure:
         *       0 -   7 : Spurious Vector
         *       8       : APIC Software Enable
         *       9       : Focus Processor Checking
         *      10 -  11 : Reserved (must be 0)
         *      12       : EOI Broadcast Suppression
         *      13 -  31 : Reserved (must be 0)
         */

        /*  Properties  */

        BITFIELD_DEFAULT_1WEx( 8, SoftwareEnabled              , 32)
        BITFIELD_DEFAULT_1WEx( 9, DisableFocusProcessorChecking, 32)
        BITFIELD_DEFAULT_1WEx(12, EoiBroadcastSuppression      , 32)

        BITFIELD_DEFAULT_2WEx( 0,  8, uint8_t, SpuriousVector  , 32)

        /*  Constructor  */

        /**
         *  Creates a new LAPIC SVR structure from the given raw value.
         */
        inline explicit LapicSvr(uint32_t const val)
        {
            this->Value = val;
        }

        /*  Field(s)  */

    private:

        uint32_t Value;
    };

    /**
     *  <summary>APIC interrupt delivery modes.</summary>
     */
    enum class InterruptDeliveryModes : unsigned
    {
        Fixed     = 0,
        Reserved1 = 1,
        SMI       = 2,
        Reserved2 = 3,
        NMI       = 4,
        Init      = 5,
        StartUp   = 6,
        Reserved3 = 7,
    };

    /**
     *  <summary>
     *  Known values for the destination shorthand field of the ICR.
     *  </summary>
     */
    enum class IcrDestinationShorthand : unsigned
    {
        None             = 0,
        Self             = 1,
        AllIncludingSelf = 2,
        AllExcludingSelf = 3,
    };

    /**
     *  <summary>
     *  Represents the contents of the Interrupt Command Register of the
     *  LAPIC.
     *  </summary>
     */
    struct LapicIcr
    {
        friend class Lapic;

        /*  Bit structure:
         *       0 -   7 : Vector
         *       8 -  10 : Delivery Mode
         *      11       : Destination Mode (1 for logical)
         *      12       : Delivery Status
         *      13       : Reserved (must be 0)
         *      14       : Level (1 for assert)
         *      15       : Trigger Mode (1 for Level)
         *      16 -  17 : Reserved (must be 0)
         *      18 -  19 : Destination Shorthand
         *      20 -  31 : Reserved (must be 0)
         *      32 -  63 : Destination
         */

        /*  Properties  */

        BITFIELD_DEFAULT_1W(11, DestinationLogical)
        BITFIELD_DEFAULT_1O(12, DeliveryStatus)
        BITFIELD_DEFAULT_1W(14, Assert)
        BITFIELD_DEFAULT_1W(15, LevelTriggered)

        BITFIELD_DEFAULT_2W( 0,  8, uint8_t                , Vector              )
        BITFIELD_DEFAULT_4W( 8,  3, InterruptDeliveryModes , DeliveryMode        )
        BITFIELD_DEFAULT_4W(18,  2, IcrDestinationShorthand, DestinationShorthand)
        BITFIELD_DEFAULT_4W(32, 32, uint32_t               , Destination         )

        inline auto SetVector(KnownIsrs const val) -> decltype(*this)
        {
            return this->SetVector((uint8_t)val);
        }

        /*  Constructor  */

        /**
         *  Creates a new LAPIC ICR structure from the given raw value.
         */
        inline explicit constexpr LapicIcr(uint64_t const val)
            : Value(val)
        {
            
        }

        /**
         *  Creates a new LAPIC ICR structure from the given low and high values.
         */
        inline constexpr LapicIcr(uint32_t const low, uint32_t const high)
            : Low(low), High(high)
        {
            
        }

        /*  Field(s)  */

    private:

        __extension__ union
        {
            uint64_t Value;

            struct
            {
                uint32_t Low;
                uint32_t High;
            };
        };
    };

    /**
     * <summary>Represents possible APIC timer modes.</summary>
     */
    enum class ApicTimerMode : unsigned
    {
        OneShot     = 0x00,
        Periodic    = 0x01,
        TscDeadline = 0x02,
    };

    /**
     *  <summary>
     *  Represents the contents of the APIC timer LVT register.
     *  </summary>
     */
    struct ApicTimerLvt
    {
        friend class Lapic;

        /*  Bit structure:
         *       0 -   7 : Vector
         *       8 -  11 : Reserved (must be 0)
         *      12       : NO IDEA
         *      13 -  15 : Reserved (must be 0)
         *      16       : Mask
         *      17 -  18 : Timer Mode
         *      19 -  31 : Reserved (must be 0)
         */

        /*  Properties  */

        BITFIELD_DEFAULT_1WEx( 8, NoIdea                   , 32)
        BITFIELD_DEFAULT_1WEx(16, Mask                     , 32)

        BITFIELD_DEFAULT_2WEx( 0,  8, uint8_t      , Vector, 32)
        BITFIELD_DEFAULT_4WEx(17,  2, ApicTimerMode, Mode  , 32)

        inline auto SetVector(KnownIsrs const val) -> decltype(*this)
        {
            return this->SetVector((uint8_t)val);
        }

        /*  Constructor  */

        /**
         *  Creates a new APIC Timer LVT structure from the given raw value.
         */
        inline explicit ApicTimerLvt(uint32_t const val)
        {
            this->Value = val;
        }

        /*  Field(s)  */

    private:

        uint32_t Value;
    };

    /**
     *  <summary>Contains methods for interacting with the local APIC.</summary>
     */
    class Lapic
    {
    public:
        /*  Statics  */

        static __thread bool X2ApicMode;

        /*  Addresses  */

        static paddr_t PhysicalAddress;
        static vaddr_t const VirtualAddress;
        //  Very last page - why not?

        /*  Ender  */

        static InterruptEnderNode Ender;

        /*  Constructor(s)  */

    protected:
        Lapic() = default;

    public:
        Lapic(Lapic const &) = delete;
        Lapic & operator =(Lapic const &) = delete;

        /*  Initialization  */

        static __cold Handle Initialize();

        /*  Registers  */

        static __hot uint32_t ReadRegister(LapicRegister const reg);
        static __hot void WriteRegister(LapicRegister const reg, uint32_t const value);

        static __hot void WriteTimerLvt(ApicTimerLvt const val)
        {
            return WriteRegister(LapicRegister::TimerLvt, val.Value);
        }

        /*  Shortcuts  */

        static __forceinline uint32_t GetId()
        {
            return ReadRegister(LapicRegister::LapicId);
        }

        static __hot __forceinline void EndOfInterrupt()
        {
            WriteRegister(LapicRegister::EndOfInterrupt, 0);
        }

        static void SendIpi(LapicIcr icr);

        LAPICREGFUNC1(SpuriousInterruptVector, Svr, LapicSvr)
    };
}}}
