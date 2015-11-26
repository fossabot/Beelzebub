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
#include <memory/manager_amd64.hpp>

#include <kernel.hpp>
#include <system/cpu.hpp>

#include <math.h>
#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::Execution;
using namespace Beelzebub::Memory;
using namespace Beelzebub::System;

uint8_t * executable;
ElfProgramHeader_64 * segments;
size_t segmentCount;
uintptr_t entryPoint;

Thread testThread;
Process testProcess;
MemoryManagerAmd64 testMm;
VirtualAllocationSpace testVas;
uintptr_t const userStackBottom = 0x40000000;
uintptr_t const userStackTop = 0x40000000 + PageSize;

__cold uint8_t * AllocateTestRegion();
__cold void * JumpToRing3(void *);

Handle HandleLoadtest(size_t const index
                    , jg_info_module_t const * const module
                    , const vaddr_t vaddr
                    , size_t const size)
{
    uint8_t * const addr = reinterpret_cast<uint8_t *>(vaddr);
    size_t const len = module->length;

    executable = addr;

    msg("%n");

    ElfHeader1 * eh1 = reinterpret_cast<ElfHeader1 *>(addr);

    msg("Identification:%n");

    msg("\tMagic number: %S%n", (size_t)4, &(eh1->Identification.MagicNumber));

    msg("\tClass: %u8 (%s)%n"
        "\tData Encoding: %u8 (%s)%n"
        "\tVersion: %u8%n"
        "\tOS ABI: %u8 (%s)%n"
        "\tABI Version: %u8%n"
        , (uint64_t)(eh1->Identification.Class       ), EnumToString(eh1->Identification.Class       )
        , (uint64_t)(eh1->Identification.DataEncoding), EnumToString(eh1->Identification.DataEncoding)
        , (uint64_t)(eh1->Identification.Version     )
        , (uint64_t)(eh1->Identification.OsAbi       ), EnumToString(eh1->Identification.OsAbi       )
        , (uint64_t)(eh1->Identification.AbiVersion  )
    );

    msg("Type: %u8 (%s)%n"
        "Machine: %u8 (%s)%n"
        "Version: %u8%n"
        , (uint64_t)(eh1->Type), EnumToString(eh1->Type)
        , (uint64_t)(eh1->Machine), EnumToString(eh1->Machine)
        , (uint64_t)(eh1->Version)
    );

    ASSERT(eh1->Identification.Class == ElfClass::Elf64
        , "Expected class ELF64!");
    ASSERT(eh1->Identification.DataEncoding == ElfDataEncoding::LittleEndian
        , "Expected class ELF64!");

    ElfHeader2_64 * eh2 = reinterpret_cast<ElfHeader2_64 *>(addr + sizeof(ElfHeader1));

    msg("Entry Point                : %X8%n"
        "Program Header Table Offset: %X8%n"
        "Section Header Table Offset: %X8%n"
        , eh2->EntryPoint
        , eh2->ProgramHeaderTableOffset
        , eh2->SectionHeaderTableOffset
    );

    entryPoint = eh2->EntryPoint;

    ElfHeader3 * eh3 = reinterpret_cast<ElfHeader3 *>(addr + sizeof(ElfHeader1) + sizeof(ElfHeader2_64));

    msg("Flags:   %X4%n"
        "Header Size: %X2 (%u2)%n"
        "Program Header Table Entry Size: %u2%n"
        "Program Header Table Entry Count: %u2%n"
        "Section Header Table Entry Size: %u2%n"
        "Section Header Table Entry Count: %u2%n"
        "Section Name String Table Index: %u2%n"
        , eh3->Flags
        , eh3->HeaderSize, eh3->HeaderSize
        , eh3->ProgramHeaderTableEntrySize
        , eh3->ProgramHeaderTableEntryCount
        , eh3->SectionHeaderTableEntrySize
        , eh3->SectionHeaderTableEntryCount
        , eh3->SectionNameStringTableIndex
    );

    ASSERT(eh3->SectionHeaderTableEntrySize == sizeof(ElfSectionHeader_64)
        , "Unusual section header type: %us; expected %us."
        , (size_t)(eh3->SectionHeaderTableEntrySize), sizeof(ElfSectionHeader_64));

    msg("%nSections:%n");

    ElfSectionHeader_64 * sectionCursor = reinterpret_cast<ElfSectionHeader_64 *>(
        addr + eh2->SectionHeaderTableOffset
    );

    for (size_t i = eh3->SectionHeaderTableEntryCount, j = 1; i > 0; --i, ++j, ++sectionCursor)
    {
        msg("\t#%us:%n"
            "\t\tName: %X4%n"
            "\t\tType: %u8 (%s)%n"
            "\t\tFlags:       %X8%n"
            "\t\tAddress:     %X8%n"
            "\t\tOffset:      %X8%n"
            "\t\tSize:        %X8%n"
            "\t\tLink:        %X4%n"
            "\t\tInfo:        %X4%n"
            "\t\tAlignment:   %X8%n"
            "\t\tEntrry Size: %X8%n"
            , j
            , sectionCursor->Name
            , (uint64_t)(sectionCursor->Type), EnumToString(sectionCursor->Type)
            , sectionCursor->Flags
            , sectionCursor->Address
            , sectionCursor->Offset
            , sectionCursor->Size
            , sectionCursor->Link
            , sectionCursor->Info
            , sectionCursor->AddressAlignment
            , sectionCursor->EntrySize
        );
    }

    msg("%nSegments:%n");

    ElfProgramHeader_64 * programCursor = reinterpret_cast<ElfProgramHeader_64 *>(
        addr + eh2->ProgramHeaderTableOffset
    );

    segments = programCursor;
    segmentCount = eh3->ProgramHeaderTableEntryCount;

    for (size_t i = eh3->ProgramHeaderTableEntryCount, j = 1; i > 0; --i, ++j, ++programCursor)
    {
        msg("\t#%us:%n"
            "\t\tType: %u8 (%s)%n"
            "\t\tFlags:     %X4%n"
            "\t\tOffset:    %X8%n"
            "\t\tVAddr:     %X8%n"
            "\t\tPAddr:     %X8%n"
            "\t\tVSize:     %X8%n"
            "\t\tPSize:     %X8%n"
            "\t\tAlignment: %X8%n"
            , j
            , (uint64_t)(programCursor->Type), EnumToString(programCursor->Type)
            , programCursor->Flags
            , programCursor->Offset
            , programCursor->VAddr
            , programCursor->PAddr
            , programCursor->PSize
            , programCursor->VSize
            , programCursor->Alignment
        );
    }

    msg("%n");

    return HandleResult::Okay;
}

void TestApplication()
{
    MemoryManagerAmd64 & mm = *(reinterpret_cast<MemoryManagerAmd64 *>(Cpu::GetData()->ActiveThread->Owner->Memory));

    mm.Vas->Clone(&testVas);
    //  Fork the VAS.

    new (&testMm) MemoryManagerAmd64(&testVas);
    //  Initialize a new memory manager with the given VAS.

    new (&testProcess) Process(&testMm);
    //  Initialize a new process for thread series B.

    new (&testThread) Thread(&testProcess);

    Handle res;
    PageDescriptor * desc = nullptr;
    //  Intermediate results.

    //  First, the kernel stack page.

    vaddr_t const stackVaddr = MemoryManagerAmd64::KernelHeapCursor.FetchAdd(PageSize);;
    paddr_t stackPaddr = Cpu::GetData()->DomainDescriptor->PhysicalAllocator->AllocatePage(desc);

    ASSERT(stackPaddr != nullpaddr && desc != nullptr
        , "Unable to allocate a physical page for test thread kernel stack!");

    res = testMm.MapPage(stackVaddr, stackPaddr
        , PageFlags::Global | PageFlags::Writable, desc);

    ASSERT(res.IsOkayResult()
        , "Failed to map page at %Xp (%XP) for test thread kernel stack: %H."
        , stackVaddr, stackPaddr
        , res);

    testThread.KernelStackTop = (uintptr_t)stackVaddr + PageSize;
    testThread.KernelStackBottom = (uintptr_t)stackVaddr;

    testThread.EntryPoint = &JumpToRing3;

    InitializeThreadState(&testThread);
    //  This sets up the thread so it goes directly to the entry point when switched to.

    //  Then, the userland stack page.

    stackPaddr = Cpu::GetData()->DomainDescriptor->PhysicalAllocator->AllocatePage(desc);

    ASSERT(stackPaddr != nullpaddr && desc != nullptr
        , "Unable to allocate a physical page for test thread user stack!");

    res = testMm.MapPage(userStackBottom, stackPaddr
        , PageFlags::Userland | PageFlags::Writable, desc);

    ASSERT(res.IsOkayResult()
        , "Failed to map page at %Xp (%XP) for test thread user stack: %H."
        , userStackBottom, stackPaddr
        , res);

    //  Then, the app's segments.

    ElfProgramHeader_64 * programCursor = segments;

    for (size_t i = segmentCount, j = 1; i > 0; --i, ++j, ++programCursor)
    {
        vaddr_t const segVaddr    = RoundDown(programCursor->VAddr, PageSize);
        vaddr_t const segVaddrEnd = RoundUp  (programCursor->VAddr + programCursor->VSize, PageSize);

        PageFlags pageFlags = PageFlags::Userland;

        if (0 != (programCursor->Flags & ElfProgramHeaderFlags::Writable))
            pageFlags |= PageFlags::Writable;
        if (0 != (programCursor->Flags & ElfProgramHeaderFlags::Executable))
            pageFlags |= PageFlags::Executable;

        for (vaddr_t vaddr = segVaddr; vaddr < segVaddrEnd; vaddr += PageSize)
        {
            paddr_t const paddr = Cpu::GetData()->DomainDescriptor->PhysicalAllocator->AllocatePage(desc);

            ASSERT(paddr != nullpaddr && desc != nullptr
                , "Unable to allocate a physical page for test app segment #%us!"
                , j);

            res = testMm.MapPage(vaddr, paddr, pageFlags, desc);

            ASSERT(res.IsOkayResult()
                , "Failed to map page at %Xp (%XP) for test app segment #%us: %H."
                , vaddr, paddr
                , j, res);
        }
    }

    //  Finally, a region for test incrementation.

    AllocateTestRegion();

    BootstrapThread.IntroduceNext(&testThread);
}

uint8_t * AllocateTestRegion()
{
    Handle res;
    PageDescriptor * desc = nullptr;
    //  Intermediate results.

    vaddr_t const vaddr1 = 0x30000;
    size_t const size = vaddr1;
    vaddr_t const vaddr2 = MemoryManagerAmd64::KernelHeapCursor.FetchAdd(size);
    //  Test pages.

    for (size_t offset = 0; offset < size; offset += PageSize)
    {
        paddr_t const paddr = Cpu::GetData()->DomainDescriptor->PhysicalAllocator->AllocatePage(desc);
        
        ASSERT(paddr != nullpaddr && desc != nullptr
            , "Unable to allocate a physical page for test page of process %Xp!"
            , &testProcess);

        res = testProcess.Memory->MapPage(vaddr1 + offset, paddr
            , PageFlags::Userland | PageFlags::Writable, desc);

        ASSERT(res.IsOkayResult()
            , "Failed to map page at %Xp (%XP) as test page in owning process: %H."
            , vaddr1 + offset, paddr
            , res);

        res = testMm.MapPage(vaddr2 + offset, paddr
            , PageFlags::Global | PageFlags::Writable, desc);

        ASSERT(res.IsOkayResult()
            , "Failed to map page at %Xp (%XP) as test page with boostrap memory manager: %H."
            , vaddr2 + offset, paddr
            , res);
    }

    return (uint8_t *)(uintptr_t)vaddr2;
}

void * JumpToRing3(void * arg)
{
    MSG_("About to go to ring 3!%n");

    while (true) CpuInstructions::DoNothing();
}

#endif
