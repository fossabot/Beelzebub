#pragma once

#include <system/registers_x86.hpp>
#include <memory/paging.hpp>
#include <handles.h>

namespace Beelzebub { namespace System
{
    /**
     * Represents the contents of the CR0 register.
     */
    struct Cr0
    {
        /*  Bit structure:
         *       0       : Protected Mode Enable
         *       1       : Monitor Co-processor
         *       2       : Emulation
         *       3       : Task Switched
         *       4       : Extension Type
         *       5       : Numeric Error
         *      16       : Write Protect
         *      18       : Alignment Mask
         *      29       : Not-Write Through
         *      30       : Cache Disable
         *      31       : Paging
         */

        static uint64_t const ProtectedModeEnableBitIndex    =  0;
        static uint64_t const MonitorCoprocessorBitIndex     =  1;
        static uint64_t const EmulationBitIndex              =  2;
        static uint64_t const TaskSwitchedBitIndex           =  3;
        static uint64_t const ExtensionTypeBitIndex          =  4;
        static uint64_t const NumericErrorBitIndex           =  5;
        static uint64_t const WriteProtectBitIndex           = 16;
        static uint64_t const AlignmentMaskBitIndex          = 18;
        static uint64_t const NotWriteThroughBitIndex        = 29;
        static uint64_t const CacheDisableBitIndex           = 30;
        static uint64_t const PagingBitIndex                 = 31;

        /*  Constructors  */

        /**
         *  Creates a new CR0 structure from the given raw value.
         */
        __bland __forceinline Cr0(uint64_t const val)
        {
            this->Value = val;
        }

        /**
         *  Creates a new CR0 structure with the given flags.
         */
        __bland __forceinline Cr0(bool const protectedModeEnable
                                , bool const monitorCoprocessor
                                , bool const emulation
                                , bool const taskSwitched
                                , bool const extensionType
                                , bool const numericError
                                , bool const writeProtect
                                , bool const alignmentMask
                                , bool const notWriteThrough
                                , bool const cacheDisable
                                , bool const paging)
        {
            this->Value = (protectedModeEnable ? ProtectedModeEnableBit : 0)
                        | (monitorCoprocessor  ? MonitorCoprocessorBit  : 0)
                        | (emulation           ? EmulationBit           : 0)
                        | (taskSwitched        ? TaskSwitchedBit        : 0)
                        | (extensionType       ? ExtensionTypeBit       : 0)
                        | (numericError        ? NumericErrorBit        : 0)
                        | (writeProtect        ? WriteProtectBit        : 0)
                        | (alignmentMask       ? AlignmentMaskBit       : 0)
                        | (notWriteThrough     ? NotWriteThroughBit     : 0)
                        | (cacheDisable        ? CacheDisableBit        : 0)
                        | (paging              ? PagingBit              : 0);
        }

        /*  Properties  */

        BITFIELD_FLAG_RW(ProtectedModeEnableBitIndex, ProtectedModeEnable, uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(MonitorCoprocessorBitIndex , MonitorCoprocessor , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(EmulationBitIndex          , Emulation          , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(TaskSwitchedBitIndex       , TaskSwitched       , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(ExtensionTypeBitIndex      , ExtensionType      , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(NumericErrorBitIndex       , NumericError       , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(WriteProtectBitIndex       , WriteProtect       , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(AlignmentMaskBitIndex      , AlignmentMask      , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(NotWriteThroughBitIndex    , NotWriteThrough    , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(CacheDisableBitIndex       , CacheDisable       , uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(PagingBitIndex             , Paging             , uint64_t, this->Value, __bland, const, static)

        /*  Field(s)  */

    //private:

        uint64_t Value;
    };

    /**
     * Represents the contents of the CR3 register.
     */
    struct Cr3
    {
        /*  Bit structure with PCID disabled:
         *       0 -   2 : Ignored
         *       3       : PWT (page-level write-through)
         *       4       : PCD (page-level cache disable)
         *       5 -  11 : Ignored
         *      12 - M-1 : Physical address of PML4 table; 4-KiB aligned.
         *       M -  63 : Reserved (must be 0)
         *
         *  Bit structure with PCID enabled:
         *       0 -  11 : PCID
         *      12 - M-1 : Physical address of PML4 table; 4-KiB aligned.
         *       M -  63 : Reserved (must be 0)
         */

        static uint64_t const PwtBitIndex           =  3;
        static uint64_t const PcdBitIndex           =  4;

        static uint64_t const AddressBits   = 0x000FFFFFFFFFF000ULL;
        static uint64_t const PcidBits      = 0x0000000000000FFFULL;

        /*  Constructors  */

        /**
         *  Creates a new CR3 structure from the given raw value.
         */
        __bland inline Cr3(uint64_t const val)
            : Value(val)
        {
            
        }

        /**
         *  Creates a new CR3 structure for use with PCID disabled.
         */
        __bland __forceinline Cr3(paddr_t const paddr, bool const PWT, bool const PCD)
            : Value(((uint64_t)paddr & AddressBits)
                  | (PWT ? PwtBit : 0)
                  | (PCD ? PcdBit : 0))
        {
            
        }

        /**
         *  Creates a new CR3 structure for use with PCID enabled.
         */
        __bland __forceinline Cr3(paddr_t const paddr, Handle const process)
            : Value(((uint64_t)paddr    & AddressBits)
                  | (process.GetIndex() & PcidBits   ))
        {
            //  TODO: Check whether the handle is correct or not.
        }

        /*  Properties  */

        BITFIELD_FLAG_RW(PwtBitIndex, Pwt, uint64_t, this->Value, __bland, const, static)
        BITFIELD_FLAG_RW(PcdBitIndex, Pcd, uint64_t, this->Value, __bland, const, static)

        /**
         *  Gets the physical address of the PML4 table.
         */
        __bland __forceinline Memory::Pml4 * GetPml4Ptr() const
        {
            return (Memory::Pml4 *)(this->Value & AddressBits);
        }
        /**
         *  Sets the physical address of the PML4 table.
         */
        __bland __forceinline void SetPml4Ptr(Memory::Pml4 const * const paddr)
        {
            this->Value = ((uint64_t)paddr       &  AddressBits)
                        | (          this->Value & ~AddressBits);
        }

        /**
         *  Gets the physical address of the PML4 table.
         */
        __bland __forceinline paddr_t GetAddress() const
        {
            return (paddr_t)(this->Value & AddressBits);
        }
        /**
         *  Sets the physical address of the PML4 table.
         */
        __bland __forceinline void SetAddress(paddr_t const paddr)
        {
            this->Value = ((uint64_t)paddr       &  AddressBits)
                        | (          this->Value & ~AddressBits);
        }

        /*  PCID  */

        /**
         *  Gets the process ID according to the PCID value.
         */
        __bland __forceinline Handle GetProcess() const
        {
            //  TODO: Construct proper handles here!

            return Handle(HandleType::Process, this->Value & PcidBits);
        }
        /**
         *  Sets the PCID value from the given process.
         */
        __bland __forceinline void SetProcess(Handle const process)
        {
            //  TODO: Check whether the handle is correct or not.

            this->Value = (process.GetIndex() &  PcidBits)
                        | (       this->Value & ~PcidBits);
        }

        /**
         *  Gets the PCID value.
         */
        __bland __forceinline uint64_t GetPcid() const
        {
            //  TODO: Construct proper handles here!

            return this->Value & PcidBits;
        }
        /**
         *  Sets the PCID value.
         */
        __bland __forceinline void SetPcid(uint64_t const pcid)
        {
            //  TODO: Check whether the handle is correct or not.

            this->Value = (pcid        &  PcidBits)
                        | (this->Value & ~PcidBits);
        }

        /*  Field(s)  */

    //private:

        uint64_t Value;
    };

    //  TODO: CR4
}}
