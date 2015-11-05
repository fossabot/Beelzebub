#pragma once

#include <system/interrupts.hpp>
#include <utils/bitfields.hpp>

namespace Beelzebub { namespace System { namespace InterruptControllers
{
    /**
     *  <summary>
     *  Represents the contents of the ID register of the I/O APIC.
     *  </summary>
     */
    struct IoapicIdRegister
    {
        /*  Bit structure:
         *       0 -  23 : Reserved (must be 0)
         *      24 -  27 : I/O APIC ID
         *      28 -  31 : Reserved (must be 0)
         */

        /*  Properties  */

        BITFIELD_DEFAULT_4WEx(24,  4, uint8_t, Id, 32)

        /*  Constructor  */

        /**
         *  <summary>
         *  Creates a new I/O APIC ID register structure from the given
         *  raw value.
         *  </summary>
         */
        __bland inline explicit constexpr IoapicIdRegister(uint32_t const val)
            : Value(val)
        {
            
        }

        /*  Field(s)  */

    //private:

        uint32_t Value;
    };

    /**
     *  <summary>Contains methods for interacting with the I/O APIC.</summary>
     */
    enum class IoapicRegisters : uint8_t
    {
        Id                      = 0x00,
        Version                 = 0x01,
        ArbitrationId           = 0x02,

        RedirectionTableStart   = 0x10,
    };
    
    /**
     *  <summary>Contains methods for interacting with the I/O APIC.</summary>
     */
    class Ioapic
    {
    public:
        /*  Statics  */

        static size_t const Limit = 16;
        static size_t Count;
        static Ioapic All[Limit];

        /*  Ender  */

        static __hot __bland void IrqEnder(INTERRUPT_ENDER_ARGS);

        /*  Constructor(s)  */

    protected:
        __bland inline constexpr Ioapic()
            : Id()
            , RegisterSelector()
            , RegisterWindow()
            , GlobalIrqBase()
            , VectorOffset()
        {

        }

    public:
        __bland inline constexpr Ioapic(uint8_t const id
                                      , uintptr_t const addr
                                      , uint32_t const globalBase
                                      , uint8_t const vecOff)
            : Id(id)
            , RegisterSelector(addr + 0x00)
            , RegisterWindow(addr + 0x10)
            , GlobalIrqBase(globalBase)
            , VectorOffset(vecOff)
        {

        }

        Ioapic(Ioapic const &) = delete;
        Ioapic & operator =(Ioapic const &) = delete;

        /*  (De)initialization  */

        __cold __bland void Initialize();

        /*  Fields  */

        uint8_t   const Id;
        uintptr_t const RegisterSelector;
        uintptr_t const RegisterWindow;
        uint32_t  const GlobalIrqBase;
        uint8_t   VectorOffset;
    };
}}}
