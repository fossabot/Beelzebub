/*
    Copyright (c) 2016 Alexandru-Mihai Maftei. All rights reserved.


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

#include <crt0.hpp>
// #include <syscalls.h>
#include <syscalls/memory.h>
#include <terminals/debug.hpp>
#include <kernel_data.hpp>
#include <execution/elf_default_mapper.hpp>
#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::Execution;
using namespace Beelzebub::Terminals;

/// Global constructors.
__extern __used void _init(void);

static DebugTerminal procDbgTrm;
static Elf ApplicationImage;

static bool HeaderValidator(ElfHeader1 const * header, void * data)
{
    return header->Identification.Class == ElfClass::Elf64;
}

__extern __bland __used void _start(char * args)
{
    // PerformSyscall(SyscallSelection::DebugPrint, const_cast<char *>("\r\nLIBRARY ENTRY POINT\r\n"), 0, 0, 0, 0);

    _init();

    Debug::DebugTerminal = &procDbgTrm;

    // PerformSyscall(SyscallSelection::DebugPrint, const_cast<char *>("\r\nABOUT TO TRY DEBUG_TERM\r\n"), 0, 0, 0, 0);

    DEBUG_TERM  << "Testing stream operator on the debug terminal inside the "
                << "runtime's entry point!" << EndLine;

    DEBUG_TERM  << STARTUP_DATA.RuntimeImage.GetSymbol(STARTUP_DATA_SYMBOL) << EndLine
                << STARTUP_DATA.RuntimeImage.GetSymbol("_start")            << EndLine
                << STARTUP_DATA.RuntimeImage.GetSymbol("__start")           << EndLine
                << STARTUP_DATA.RuntimeImage.GetSymbol("_init")             << EndLine
                << STARTUP_DATA.RuntimeImage.GetSymbol("_fini")             << EndLine
                << STARTUP_DATA.RuntimeImage.GetSymbol("BLEEEERGH")         << EndLine;

    DEBUG_TERM  << "Memory image start: " << (void *)(STARTUP_DATA.MemoryImageStart) << EndLine
                << "Memory image end:   " << (void *)(STARTUP_DATA.MemoryImageEnd  ) << EndLine;

    Handle res = MemoryRequest(0, 0x10000, mem_req_opts_t::Writable);
    uint64_t volatile * testPtr = (uint64_t volatile *)res.GetPage();

    ASSERT(testPtr != nullptr, "Failed memory request: %H", res);

    testPtr[0] = 42;
    testPtr[1] = 1337;
    testPtr[2] = 616;

    DEBUG_TERM << "3 numbas: " << testPtr[0] << ", " << testPtr[1] << ", " << testPtr[2] << EndLine;
    DEBUG_TERM << "@ " << (void *)testPtr << EndLine;

    //  Now to finally parse the actual application.

    new (&ApplicationImage) Elf(reinterpret_cast<void *>(STARTUP_DATA.MemoryImageStart), STARTUP_DATA.MemoryImageEnd - STARTUP_DATA.MemoryImageStart);

    ElfValidationResult evRes = ApplicationImage.ValidateAndParse(&HeaderValidator, nullptr, nullptr);

    if (evRes != ElfValidationResult::Success)
    {
        DEBUG_TERM  << "Failed to validate and parse application image: "
                    << evRes << Terminals::EndLine;

        ASSERT(false);
    }

    //  And map it.

    evRes = ApplicationImage.LoadAndValidate64(&MapSegment64, &UnmapSegment64, nullptr);

    if (evRes != ElfValidationResult::Success)
    {
        DEBUG_TERM  << "Failed to load application image: "
                    << evRes << Terminals::EndLine;

        ASSERT(false);
    }

    QuitProcess(HandleResult::Okay, 0);
}

__extern void __start(char * input) __alias(_start);
//  Just in case there's any voodoo going on.
