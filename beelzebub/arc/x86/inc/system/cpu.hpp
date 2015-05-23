#pragma once

#include <arc/memory/paging.hpp>
#include <arc/system/registers.hpp>
#include <arc/system/cpuid.hpp>
#include <metaprogramming.h>

#define REGFUNC1(regl, regu, type)                                   \
static __bland __forceinline type MCATS2(Get, regu)()                \
{                                                                    \
    type ret;                                                        \
                                                                     \
    asm volatile ( "mov %%" #regl ", %0\n\t"                         \
                 : "=a"(ret) );                                      \
                                                                     \
    return ret;                                                      \
}                                                                    \
static __bland __forceinline void MCATS2(Set, regu)(const type val)  \
{                                                                    \
    asm volatile ( "mov %0, %%" #regl "\n\t"                         \
                 : : "r"(val) );                                     \
}

#define REGFUNC2(regl, regu, type, type2)                            \
static __bland __forceinline type2 MCATS2(Get, regu)()               \
{                                                                    \
    type ret;                                                        \
                                                                     \
    asm volatile ( "mov %%" #regl ", %0\n\t"                         \
                 : "=a"(ret) );                                      \
                                                                     \
    return type2(ret);                                               \
}                                                                    \
static __bland __forceinline void MCATS2(Set, regu)(const type val)  \
{                                                                    \
    asm volatile ( "mov %0, %%" #regl "\n\t"                         \
                 : : "r"(val) );                                     \
}                                                                    \
static __bland __forceinline void MCATS2(Set, regu)(const type2 val) \
{                                                                    \
    type innerVal = val.Value;                                       \
                                                                     \
    asm volatile ( "mov %0, %%" #regl "\n\t"                         \
                 : : "r"(innerVal) );                                \
}

#define MSRFUNC1(name, prettyName, type)                                  \
static __bland __forceinline type MCATS2(Get, prettyName)()               \
{                                                                         \
    uint64_t temp = 0;                                                    \
    ReadMsr(Msr::name, temp);                                             \
    return type(temp);                                                    \
}                                                                         \
static __bland __forceinline void MCATS2(Set, prettyName)(const type val) \
{                                                                         \
    WriteMsr(Msr::name, val.Value);                                       \
}

namespace Beelzebub { namespace System
{
    /**
     *  Represents a processing unit of the system.
     */
    class Cpu
    {
    public:

        /*  Control  */

        static const bool CanHalt = true;

        static __bland __forceinline void Halt()
        {
            asm volatile ("hlt\n\t");
        }

        /*  Interrupts  */

        static __bland __forceinline bool InterruptsEnabled()
        {
            size_t flags;

            asm volatile ( "pushf\n\t"
                           "pop %0\n\t"
                           : "=r"(flags) );

            return (flags & (size_t)(1 << 9)) != 0;
        }

        static __bland __forceinline void EnableInterrupts()
        {
            asm volatile ("sti\n\t");
        }

        static __bland __forceinline void DisableInterrupts()
        {
            asm volatile ("cli\n\t");
        }

        static __bland __forceinline void LIDT(const uintptr_t base
                                             , const uint16_t size)
        {
            struct
            {
                uint16_t length;
                uintptr_t base;
            } __attribute__((packed)) IDTR;

            IDTR.length = size;
            IDTR.base = base;

            asm ( "lidt (%0)\n\t" : : "p"(&IDTR) );
        }

        /*  Far memory ops  */

        static __bland __forceinline uint32_t FarGet32(const uint16_t sel
                                                     , const uintptr_t off)
        {
            uint32_t ret;
            asm ( "push %%fs          \n\t"
                  "mov  %1, %%fs      \n\t"
                  "mov  %%fs:(%2), %0 \n\t"
                  "pop  %%fs          \n\t"
                  : "=r"(ret) : "g"(sel), "r"(off) );
            return ret;
        }

        /*  Port I/O  */

        static __bland __forceinline void Out8(const uint16_t port
                                             , const uint8_t value)
        {
            asm volatile ("outb %1, %0 \n\t" :: "dN" (port), "a" (value));
        }

        static __bland __forceinline void Out16(const uint16_t port
                                              , const uint16_t value)
        {
            asm volatile ("outw %1, %0 \n\t" :: "dN" (port), "a" (value));
        }

        static __bland __forceinline void Out32(const uint16_t port
                                              , const uint32_t value)
        {
            asm volatile ("outl %1, %0 \n\t" :: "dN" (port), "a" (value));
        }


        static __bland __forceinline void In8(const uint16_t port, uint8_t & value)
        {
            asm volatile ("inb %1, %0 \n\t" : "=a" (value) : "dN" (port));
        }

        static __bland __forceinline void In16(const uint16_t port, uint16_t & value)
        {
            asm volatile ("inw %1, %0 \n\t" : "=a" (value) : "dN" (port));
        }

        static __bland __forceinline void In32(const uint16_t port, uint32_t & value)
        {
            asm volatile ("inl %1, %0 \n\t" : "=a" (value) : "dN" (port));
        }


        static __bland __forceinline uint8_t In8(const uint16_t port)
        {
            uint8_t value;
            asm volatile ("inb %1, %0 \n\t" : "=a" (value) : "dN" (port));
            return value;
        }

        static __bland __forceinline uint16_t In16(const uint16_t port)
        {
            uint16_t value;
            asm volatile ("inw %1, %0 \n\t" : "=a" (value) : "dN" (port));
            return value;
        }

        static __bland __forceinline uint32_t In32(const uint16_t port)
        {
            uint32_t value;
            asm volatile ("inl %1, %0 \n\t" : "=a" (value) : "dN" (port));
            return value;
        }

        /*  Control Registers  */

        REGFUNC2(cr0, Cr0, size_t, Beelzebub::System::Cr0)
        REGFUNC1(cr2, Cr2, void *)
        REGFUNC2(cr3, Cr3, size_t, Beelzebub::System::Cr3)
        REGFUNC1(cr4, Cr4, size_t)

        /*  MSRs  */

        static __bland __forceinline MsrValue ReadMsr(const Msr reg)
        {
            uint32_t a, d;

            asm volatile ( "rdmsr \n\t"
                         : "=a" (a), "=d" (d)
                         : "c" (reg));

            return {{d, a}};
        }

        static __bland __forceinline void ReadMsr(const Msr reg, uint32_t & a, uint32_t & d)
        {
            asm volatile ( "rdmsr \n\t"
                         : "=a" (a), "=d" (d)
                         : "c" (reg));
        }

        static __bland __forceinline void ReadMsr(const Msr reg, uint64_t & val)
        {
            uint32_t a, d;

            asm volatile ( "rdmsr \n\t"
                         : "=a" (a), "=d" (d)
                         : "c" (reg));

            val = (uint64_t)a | ((uint64_t)d << 32);
        }

        static __bland __forceinline void WriteMsr(const Msr reg, const MsrValue val)
        {
            asm volatile ( "wrmsr \n\t"
                         : 
                         : "c" (reg), "a" (val.Dwords.Low), "d" (val.Dwords.High));
        }

        static __bland __forceinline void WriteMsr(const Msr reg, const uint64_t val)
        {
            register uint32_t a asm("eax") = (uint32_t)val;
            register uint32_t d asm("edx") = (uint32_t)(val >> 32);

            asm volatile ( "wrmsr \n\t"
                         : 
                         : "c" (reg), "a" (a), "d" (d));
        }

        MSRFUNC1(IA32_EFER, EFER, Ia32Efer)

        /*  Shortcuts  */

        static __bland __forceinline void EnableNxBit()
        {
            const Msr reg = Msr::IA32_EFER;

            asm volatile ( "rdmsr           \n\t"
                           "or $2048, %%eax \n\t" //  That be bit 11.
                           "wrmsr           \n\t"
                         :
                         : "c" (reg)
                         : "eax", "edx");
        }

        static __bland size_t GetUnpreciseIndex();
    };
}}
