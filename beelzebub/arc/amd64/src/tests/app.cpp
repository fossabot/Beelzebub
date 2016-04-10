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

#ifdef __BEELZEBUB__TEST_APP

#include <tests/app.hpp>
#include <execution/elf.hpp>
#include <execution/thread.hpp>
#include <execution/thread_init.hpp>
#include <execution/ring_3.hpp>
#include <memory/vmm.hpp>

#include <kernel.hpp>
#include <entry.h>
#include <system/cpu.hpp>

#include <string.h>
#include <math.h>
#include <debug.hpp>
#include <_print/paging.hpp>

using namespace Beelzebub;
using namespace Beelzebub::Execution;
using namespace Beelzebub::Memory;
using namespace Beelzebub::Synchronization;
using namespace Beelzebub::System;
using namespace Beelzebub::Terminals;

static uint8_t * executable;
static uintptr_t entryPoint;

static ElfProgramHeader_64 * segments;
static size_t segmentCount;

static ElfRelaEntry_64 * relocationsA, * relocationsJ;
static size_t relocationsACount, relocationsJCount;

static ElfSymbol_64 * symbols;

static uintptr_t executable_base = 0x1000000;

static Thread testThread;
static Thread testWatcher;
static Process testProcess;
static uintptr_t const userStackBottom = 0x40000000;
static uintptr_t const userStackTop = 0x40000000 + PageSize;

static __cold void * JumpToRing3(void *);
static __cold void * WatchTestThread(void *);

Handle HandleLoadtest(size_t const index
                    , jg_info_module_t const * const module
                    , const vaddr_t vaddr
                    , size_t const size)
{
    uint8_t * const addr = reinterpret_cast<uint8_t *>(vaddr);
    // size_t const len = module->length;

    // executable = addr;

    DEBUG_TERM << EndLine
        << "############################## LOADTEST APP START ##############################"
        << EndLine;

    ElfHeader1 * eh1 = reinterpret_cast<ElfHeader1 *>(addr);

    DEBUG_TERM << *eh1 << EndLine;

    ASSERT(eh1->Identification.Class == ElfClass::Elf64);
    ASSERT(eh1->Identification.DataEncoding == ElfDataEncoding::LittleEndian);

    ElfHeader2_64 * eh2 = reinterpret_cast<ElfHeader2_64 *>(addr + sizeof(ElfHeader1));

    DEBUG_TERM << Hexadecimal << *eh2 << Decimal << EndLine;

    // entryPoint = eh2->EntryPoint;

    ElfHeader3 * eh3 = reinterpret_cast<ElfHeader3 *>(addr + sizeof(ElfHeader1) + sizeof(ElfHeader2_64));

    DEBUG_TERM << *eh3 << EndLine;

    ASSERT(eh3->SectionHeaderTableEntrySize == sizeof(ElfSectionHeader_64)
        , "Unusual section header type: %us; expected %us."
        , (size_t)(eh3->SectionHeaderTableEntrySize), sizeof(ElfSectionHeader_64));

    // msg("%nSections:%n");

    // ElfSectionHeader_64 * sectionCursor = reinterpret_cast<ElfSectionHeader_64 *>(
    //     addr + eh2->SectionHeaderTableOffset
    // );

    // char const * sectionNames = reinterpret_cast<char const *>(addr + sectionCursor[eh3->SectionNameStringTableIndex].Offset);

    // for (size_t i = eh3->SectionHeaderTableEntryCount, j = 1; i > 0; --i, ++j, ++sectionCursor)
    // {
    //     msg("\t#%us:%n"
    //         "\t\tName: %X4 (%s)%n"
    //         "\t\tType: %u8 (%s)%n"
    //         "\t\tFlags:       %X8%n"
    //         "\t\tAddress:     %X8%n"
    //         "\t\tOffset:      %X8%n"
    //         "\t\tSize:        %X8%n"
    //         "\t\tLink:        %X4%n"
    //         "\t\tInfo:        %X4%n"
    //         "\t\tAlignment:   %X8%n"
    //         "\t\tEntrry Size: %X8%n"
    //         , j
    //         , sectionCursor->Name, sectionNames + sectionCursor->Name
    //         , (uint64_t)(sectionCursor->Type), EnumToString(sectionCursor->Type)
    //         , sectionCursor->Flags
    //         , sectionCursor->Address
    //         , sectionCursor->Offset
    //         , sectionCursor->Size
    //         , sectionCursor->Link
    //         , sectionCursor->Info
    //         , sectionCursor->AddressAlignment
    //         , sectionCursor->EntrySize
    //     );
    // }

    DEBUG_TERM << EndLine << "Segments:" << EndLine;

    ElfProgramHeader_64 * programCursor = reinterpret_cast<ElfProgramHeader_64 *>(
        addr + eh2->ProgramHeaderTableOffset
    );

    // segments = programCursor;
    // segmentCount = eh3->ProgramHeaderTableEntryCount;

    for (size_t i = eh3->ProgramHeaderTableEntryCount, j = 1; i > 0; --i, ++j, ++programCursor)
    {
        // msg("\t#%us:%n"
        //     "\t\tType: %u8 (%s)%n"
        //     "\t\tFlags:     %X4%n"
        //     "\t\tOffset:    %X8%n"
        //     "\t\tVAddr:     %X8%n"
        //     "\t\tPAddr:     %X8%n"
        //     "\t\tVSize:     %X8%n"
        //     "\t\tPSize:     %X8%n"
        //     "\t\tAlignment: %X8%n"
        //     , j
        //     , (uint64_t)(programCursor->Type), EnumToString(programCursor->Type)
        //     , programCursor->Flags
        //     , programCursor->Offset
        //     , programCursor->VAddr
        //     , programCursor->PAddr
        //     , programCursor->PSize
        //     , programCursor->VSize
        //     , programCursor->Alignment
        // );

        DEBUG_TERM << "#" << j << ": " << *programCursor << EndLine;

        if (programCursor->Type == ElfProgramHeaderType::Dynamic)
        {
            msg("\tEntries:%n");

            ElfDynamicEntry_64 * dynEntCursor = reinterpret_cast<ElfDynamicEntry_64 *>(
                addr + programCursor->Offset
            );
            size_t offset = 0, k = 0;

            do
            {
                DEBUG_TERM << "\t#" << k << ": " << *dynEntCursor << EndLine;

                ++dynEntCursor;
                offset += sizeof(ElfDynamicEntry_64);
                ++k;
            } while (offset < programCursor->PSize && dynEntCursor->Tag != DT_NULL);
        }
    }

    DEBUG_TERM << EndLine
        << "############################### LOADTEST APP END ###############################"
        << EndLine;

    return HandleResult::Okay;
}

Handle HandleRuntimeLib(size_t const index
                      , jg_info_module_t const * const module
                      , const vaddr_t vaddr
                      , size_t const size)
{
    uint8_t * const addr = reinterpret_cast<uint8_t *>(vaddr);
    // size_t const len = module->length;

    executable = addr;

    DEBUG_TERM << EndLine
        << "############################## RUNTIME LIB  START ##############################"
        << EndLine;

    ElfHeader1 * eh1 = reinterpret_cast<ElfHeader1 *>(addr);

    DEBUG_TERM << *eh1 << EndLine;

    ASSERT(eh1->Identification.Class == ElfClass::Elf64);
    ASSERT(eh1->Identification.DataEncoding == ElfDataEncoding::LittleEndian);

    ElfHeader2_64 * eh2 = reinterpret_cast<ElfHeader2_64 *>(addr + sizeof(ElfHeader1));

    DEBUG_TERM << Hexadecimal << *eh2 << Decimal << EndLine;

    entryPoint = eh2->EntryPoint;

    ElfHeader3 * eh3 = reinterpret_cast<ElfHeader3 *>(addr + sizeof(ElfHeader1) + sizeof(ElfHeader2_64));

    DEBUG_TERM << *eh3 << EndLine;

    ASSERT(eh3->SectionHeaderTableEntrySize == sizeof(ElfSectionHeader_64)
        , "Unusual section header type: %us; expected %us."
        , (size_t)(eh3->SectionHeaderTableEntrySize), sizeof(ElfSectionHeader_64));

    DEBUG_TERM << EndLine << "Segments:" << EndLine;

    ElfProgramHeader_64 * programCursor = reinterpret_cast<ElfProgramHeader_64 *>(
        addr + eh2->ProgramHeaderTableOffset
    );

    segments = programCursor;
    segmentCount = eh3->ProgramHeaderTableEntryCount;

    uint64_t rela_offset = 0, rela_size = 0, rela_entry_size = 0;
    uint64_t relj_offset = 0, relj_size = 0;
    uint64_t symtab_offset = 0, symtab_entry_size = 0;
    uint64_t strtab_offset = 0, strtab_size = 0;

    for (size_t i = eh3->ProgramHeaderTableEntryCount, j = 1; i > 0; --i, ++j, ++programCursor)
    {
        DEBUG_TERM << "#" << j << ": " << *programCursor << EndLine;

        if (programCursor->Type == ElfProgramHeaderType::Dynamic)
        {
            msg("\tEntries:%n");

            ElfDynamicEntry_64 * dynEntCursor = reinterpret_cast<ElfDynamicEntry_64 *>(
                addr + programCursor->Offset
            );
            size_t offset = 0, k = 0;

            do
            {
                DEBUG_TERM << "\t#" << k << ": " << *dynEntCursor << EndLine;

                #define TAG_CASE(type, var) \
                    case ElfDynamicEntryTag::type: \
                        var = dynEntCursor->Value; \
                        break;

                switch (dynEntCursor->Tag)
                {
                    TAG_CASE(DT_RELA, rela_offset)
                    TAG_CASE(DT_RELASZ, rela_size)
                    TAG_CASE(DT_RELAENT, rela_entry_size)

                    TAG_CASE(DT_SYMTAB, symtab_offset)
                    TAG_CASE(DT_SYMENT, symtab_entry_size)

                    TAG_CASE(DT_STRTAB, strtab_offset)
                    TAG_CASE(DT_STRSZ, strtab_size)

                    TAG_CASE(DT_JMPREL, relj_offset)
                    TAG_CASE(DT_PLTRELSZ, relj_size)
                    
                default: break;
                }

                ++dynEntCursor;
                offset += sizeof(ElfDynamicEntry_64);
                ++k;
            } while (offset < programCursor->PSize && dynEntCursor->Tag != DT_NULL);
        }
    }

    DEBUG_TERM << "STRTAB: Offset " << Hexadecimal << strtab_offset << Decimal << ", Size " << strtab_size << EndLine;

    DEBUG_TERM << Hexadecimal << "RELA: Offset " << rela_offset << ", Size " << Decimal << rela_size << ", Entry Size " << rela_entry_size << EndLine;
    ASSERT(rela_entry_size == sizeof(ElfRelaEntry_64));

    size_t const rela_entry_count = rela_size / rela_entry_size;
    DEBUG_TERM << "RELA: Entry Count " << rela_entry_count << EndLine;

    relocationsA = reinterpret_cast<ElfRelaEntry_64 *>(addr + rela_offset);
    relocationsACount = rela_entry_count;

    // DEBUG_TERM << "Relocations:" << EndLine;

    // for (size_t i = 0; i < rela_entry_count; ++i)
    // {
    //     ElfRelaEntry_64 * rela = reinterpret_cast<ElfRelaEntry_64 *>(addr + rela_offset) + i;

    //     DEBUG_TERM << "#" << i << ": " << *rela << EndLine;
    // }

    DEBUG_TERM << Hexadecimal << "RELJ: Offset " << relj_offset << ", Size " << Decimal << relj_size << EndLine;

    size_t const relj_entry_count = relj_size / sizeof(ElfRelaEntry_64);
    DEBUG_TERM << "RELJ: Entry Count " << relj_entry_count << EndLine;

    relocationsJ = reinterpret_cast<ElfRelaEntry_64 *>(addr + relj_offset);
    relocationsJCount = relj_entry_count;

    // DEBUG_TERM << "Jump Relocations:" << EndLine;

    // for (size_t i = 0; i < relj_entry_count; ++i)
    // {
    //     ElfRelaEntry_64 * rela = reinterpret_cast<ElfRelaEntry_64 *>(addr + relj_offset) + i;

    //     DEBUG_TERM << "#" << i << ": " << *rela << EndLine;
    // }

    DEBUG_TERM << Hexadecimal << "SYMTAB: Offset " << symtab_offset << ", Entry Size " << Decimal << symtab_entry_size << EndLine;
    ASSERT(symtab_entry_size == sizeof(ElfSymbol_64));

    symbols = reinterpret_cast<ElfSymbol_64 *>(addr + symtab_offset);

    // DEBUG_TERM << "Symbols:" << EndLine;

    // for (size_t i = 0; i < 90; ++i)
    // {
    //     ElfSymbol_64 * sym = reinterpret_cast<ElfSymbol_64 *>(addr + symtab_offset + i * symtab_entry_size);

    //     DEBUG_TERM  << "#" << i << ": " << reinterpret_cast<char *>(addr + strtab_offset + sym->Name) << EndLine
    //                 << *sym << EndLine;
    // }

    DEBUG_TERM << EndLine
        << "############################### RUNTIME LIB  END ###############################"
        << EndLine;

    return HandleResult::Okay;
}

Spinlock<> TestRegionLock;

void TestApplication()
{
    TestRegionLock.Acquire();

    DEBUG_TERM << "Library will be loaded with base " << Hexadecimal << executable_base << Decimal << "." << EndLine;

    new (&testProcess) Process();
    //  Initialize a new process for thread series B.

    Vmm::Initialize(&testProcess);

    new (&testThread) Thread(&testProcess);
    new (&testWatcher) Thread(&testProcess);

    Handle res;

    //  Firstly, the kernel stack page of the test thread.

    uintptr_t stackVaddr;

    res = Vmm::AllocatePages(CpuDataSetUp ? Cpu::GetData()->ActiveThread->Owner : &BootstrapProcess
        , 3, MemoryAllocationOptions::Commit | MemoryAllocationOptions::VirtualKernelHeap
        , MemoryFlags::Global | MemoryFlags::Writable, stackVaddr);

    ASSERT(res.IsOkayResult()
        , "Failed to allocate stack for test userland thread: %H."
        , res);

    testThread.KernelStackTop = stackVaddr + 3 * PageSize;
    testThread.KernelStackBottom = stackVaddr;

    testThread.EntryPoint = &JumpToRing3;

    InitializeThreadState(&testThread);
    //  This sets up the thread so it goes directly to the entry point when switched to.

    withInterrupts (false)
        BootstrapThread.IntroduceNext(&testThread);

    //  Secondly, the kernel stack page of the watcher thread.

    res = Vmm::AllocatePages(CpuDataSetUp ? Cpu::GetData()->ActiveThread->Owner : &BootstrapProcess
        , 3, MemoryAllocationOptions::Commit | MemoryAllocationOptions::VirtualKernelHeap
        , MemoryFlags::Global | MemoryFlags::Writable, stackVaddr);

    ASSERT(res.IsOkayResult()
        , "Failed to allocate stack for test watcher thread: %H."
        , res);

    testWatcher.KernelStackTop = stackVaddr + 3 * PageSize;
    testWatcher.KernelStackBottom = stackVaddr;

    testWatcher.EntryPoint = &WatchTestThread;

    InitializeThreadState(&testWatcher);
    //  This sets up the thread so it goes directly to the entry point when switched to.

    withInterrupts (false)
        testThread.IntroduceNext(&testWatcher);
}

__cold uint8_t * AllocateTestRegion()
{
    Handle res;
    PageDescriptor * desc = nullptr;
    //  Intermediate results.

    vaddr_t const vaddr1 = 0x30000;
    size_t const size = vaddr1;
    vaddr_t const vaddr2 = Vmm::KernelHeapCursor.FetchAdd(size);
    //  Test pages.

    for (size_t offset = 0; offset < size; offset += PageSize)
    {
        paddr_t const paddr = Cpu::GetData()->DomainDescriptor->PhysicalAllocator->AllocatePage(desc);
        
        ASSERT(paddr != nullpaddr && desc != nullptr
            , "Unable to allocate a physical page for test page of process %Xp!"
            , &testProcess);

        res = Vmm::MapPage(&testProcess, vaddr1 + offset, paddr
            , MemoryFlags::Userland | MemoryFlags::Writable, desc);

        ASSERT(res.IsOkayResult()
            , "Failed to map page at %Xp (%XP) as test page in owning process: %H."
            , vaddr1 + offset, paddr
            , res);

        res = Vmm::MapPage(&testProcess, vaddr2 + offset, paddr
            , MemoryFlags::Global | MemoryFlags::Writable, desc);

        ASSERT(res.IsOkayResult()
            , "Failed to map page at %Xp (%XP) as test page with boostrap memory manager: %H."
            , vaddr2 + offset, paddr
            , res);
    }

    TestRegionLock.Release();

    return (uint8_t *)(uintptr_t)vaddr2;
}

void * JumpToRing3(void * arg)
{
    Handle res;
    PageDescriptor * desc = nullptr;
    //  Intermediate results.

    //  ... then, the userland stack page.

    paddr_t const stackPaddr = Cpu::GetData()->DomainDescriptor->PhysicalAllocator->AllocatePage(desc);

    ASSERT(stackPaddr != nullpaddr && desc != nullptr
        , "Unable to allocate a physical page for test thread user stack!");

    res = Vmm::MapPage(&testProcess, userStackBottom, stackPaddr
        , MemoryFlags::Userland | MemoryFlags::Writable, desc);

    ASSERT(res.IsOkayResult()
        , "Failed to map page at %Xp (%XP) for test thread user stack: %H."
        , userStackBottom, stackPaddr
        , res);

    //  Then, the app's segments.

    ElfProgramHeader_64 * programCursor = segments;

    for (size_t i = segmentCount, j = 1; i > 0; --i, ++j, ++programCursor)
    {
        if (programCursor->Type != ElfProgramHeaderType::Load)
            continue;

        vaddr_t const segVaddr    = executable_base + RoundDown(programCursor->VAddr, PageSize);
        vaddr_t const segVaddrEnd = executable_base + RoundUp  (programCursor->VAddr + programCursor->VSize, PageSize);

        MemoryFlags pageFlags = MemoryFlags::Userland;

        if (0 != (programCursor->Flags & ElfProgramHeaderFlags::Writable))
            pageFlags |= MemoryFlags::Writable;
        if (0 != (programCursor->Flags & ElfProgramHeaderFlags::Executable))
            pageFlags |= MemoryFlags::Executable;

        for (vaddr_t vaddr = segVaddr; vaddr < segVaddrEnd; vaddr += PageSize)
        {
            paddr_t const paddr = Cpu::GetData()->DomainDescriptor->PhysicalAllocator->AllocatePage(desc);

            ASSERT(paddr != nullpaddr && desc != nullptr
                , "Unable to allocate a physical page for test app segment #%us!"
                , j);

            res = Vmm::MapPage(&testProcess, vaddr, paddr, pageFlags, desc);

            ASSERT(res.IsOkayResult()
                , "Failed to map page at %Xp (%XP) for test app segment #%us: %H."
                , vaddr, paddr
                , j, res);
        }

        memcpy(reinterpret_cast<void *>(executable_base + programCursor->VAddr)
            , executable + programCursor->Offset, programCursor->PSize);

        if (programCursor->VSize > programCursor->PSize)
            memset(reinterpret_cast<void *>(executable_base + programCursor->VAddr + programCursor->PSize)
                , 0, programCursor->VSize - programCursor->PSize);
    }

    //  Now let's apply some relocations...

    DEBUG_TERM_ << "APPLYING RELA RELOCATIONS" << EndLine;

    for (size_t i = 0; i < relocationsACount; ++i)
    {
        ElfRelaEntry_64 * rela = relocationsA + i;

        switch (rela->Info.GetType())
        {
        case R_AMD64_RELATIVE:
            {
                uint64_t * const addr = reinterpret_cast<uint64_t *>(executable_base + rela->Offset);
                uint64_t const oldVal = *addr;

                *addr = executable_base + rela->Append;

                DEBUG_TERM_ << "RELOCATING RELA " << (void *)(addr) << " FROM " << Hexadecimal << oldVal << " INTO "
                            << *addr << "; " << executable_base << " + " << rela->Append
                            << Decimal << EndLine;
            }
            break;

        case R_AMD64_GLOB_DAT:
        case R_AMD64_JUMP_SLOT:
            {
                uint64_t * const addr = reinterpret_cast<uint64_t *>(executable_base + rela->Offset);
                uint64_t const oldVal = *addr;

                *addr = symbols[rela->Info.GetSymbol()].Value;

                DEBUG_TERM_ << "RELOCATING GDAT " << (void *)(addr) << " FROM " << Hexadecimal << oldVal << " INTO "
                            << *addr << Decimal << EndLine;
            }
            break;

        case R_AMD64_64:
            {
                uint64_t * const addr = reinterpret_cast<uint64_t *>(executable_base + rela->Offset);
                uint64_t const oldVal = *addr;

                *addr = symbols[rela->Info.GetSymbol()].Value + rela->Append;

                DEBUG_TERM_ << "RELOCATING  64  " << (void *)(addr) << " FROM " << Hexadecimal << oldVal << " INTO "
                            << *addr << "; " << symbols[rela->Info.GetSymbol()].Value
                            << " + " << rela->Append << Decimal << EndLine;
            }
            break;

        default: break; //  A warning should be emitted, perhaps?
        }
    }

    DEBUG_TERM_ << "APPLYING JUMP TABLE RELOCATIONS" << EndLine;

    for (size_t i = 0; i < relocationsJCount; ++i)
    {
        ElfRelaEntry_64 * rela = relocationsJ + i;

        switch (rela->Info.GetType())
        {
        case R_AMD64_RELATIVE:
            {
                uint64_t * const addr = reinterpret_cast<uint64_t *>(executable_base + rela->Offset);
                uint64_t const oldVal = *addr;

                *addr = executable_base + rela->Append;

                DEBUG_TERM_ << "RELOCATING RELA " << (void *)(addr) << " FROM " << Hexadecimal << oldVal << " INTO "
                            << *addr << "; " << executable_base << " + " << rela->Append
                            << Decimal << EndLine;
            }
            break;

        case R_AMD64_GLOB_DAT:
        case R_AMD64_JUMP_SLOT:
            {
                uint64_t * const addr = reinterpret_cast<uint64_t *>(executable_base + rela->Offset);
                uint64_t const oldVal = *addr;

                *addr = symbols[rela->Info.GetSymbol()].Value;

                DEBUG_TERM_ << "RELOCATING JUMP " << (void *)(addr) << " FROM " << Hexadecimal << oldVal << " INTO "
                            << *addr << Decimal << EndLine;
            }
            break;

        case R_AMD64_64:
            {
                uint64_t * const addr = reinterpret_cast<uint64_t *>(executable_base + rela->Offset);
                uint64_t const oldVal = *addr;

                *addr = symbols[rela->Info.GetSymbol()].Value + rela->Append;

                DEBUG_TERM_ << "RELOCATING  64  " << (void *)(addr) << " FROM " << Hexadecimal << oldVal << " INTO "
                            << *addr << "; " << symbols[rela->Info.GetSymbol()].Value
                            << " + " << rela->Append << Decimal << EndLine;
            }
            break;

        default: break; //  A warning should be emitted, perhaps?
        }
    }

    //  Finally, a region for test incrementation.

    AllocateTestRegion();

    MSG_("About to go to ring 3!%n");

    //while (true) CpuInstructions::Halt();

    CpuInstructions::InvalidateTlb(reinterpret_cast<void const *>(executable_base + entryPoint));

    return GoToRing3_64(executable_base + entryPoint, userStackTop);
}

void * WatchTestThread(void *)
{
    TestRegionLock.Spin();

    uint8_t  const * const data  = reinterpret_cast<uint8_t  const *>(0x30008);
    uint64_t const * const data2 = reinterpret_cast<uint64_t const *>(0x30000);

    while (true)
    {
        void * activeThread = Cpu::GetData()->ActiveThread;

        // MSG("WATCHER (%Xp) sees %u1 & %u8!%n", activeThread, *data, *data2);
        // DEBUG_TERM  << "WATCHER (" << Hexadecimal << activeThread << Decimal
        //             << ") sees " << *data << " & " << *data2 << "!" << EndLine;
    }
}

#endif
