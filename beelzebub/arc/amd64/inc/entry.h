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

#include <beel/handles.h>
#include <jegudiel.h>

#ifdef __cplusplus
    #include <terminals/base.hpp>
    #include <system/cpuid.hpp>
    #include <system/domain.hpp>

    #define Handle Beelzebub::Handle
    #define TerminalBase Beelzebub::Terminals::TerminalBase
#else
    #define TerminalBase void
    //  Hue hue hue.
    //  Hue.
#endif

#define JG_INFO_ROOT_BASE          (0xFFFFFFFFFFFE0000)

#define JG_INFO_OFFSET_EX(name)    ((uintptr_t)(JG_INFO_ROOT_BASE + JG_INFO_ROOT_EX->MCATS(name, _offset)))

#define JG_INFO_ROOT_EX            ((jg_info_root_t   *) JG_INFO_ROOT_BASE        )
#define JG_INFO_MMAP_EX            ((jg_info_mmap_t   *) JG_INFO_OFFSET_EX(mmap  ))
#define JG_INFO_MODULE_EX          ((jg_info_module_t *) JG_INFO_OFFSET_EX(module))
#define JG_INFO_STRING_EX          ((char             *) JG_INFO_OFFSET_EX(string))

__extern __public __startup void kmain_bsp();
__extern __startup void kmain_ap();

#ifdef __cplusplus
namespace Beelzebub
{
    extern System::CpuId BootstrapCpuid;
}
#endif

__extern __startup Handle ParseKernelArguments();

__extern __startup Handle InitializeInterrupts();

// TODO: Don't depend on Jegudiel; let Jegudiel depend on Beelzebub!
__extern __startup Handle InitializePhysicalAllocator(jg_info_mmap_t * map
    , size_t cnt, uintptr_t freeStart, Beelzebub::System::Domain * domain);
__extern __startup Handle InitializePhysicalMemory();
__extern __startup Handle InitializeVirtualMemory();

__extern __startup Handle InitializeAcpiTables();

__extern __startup Handle InitializeApic();

__extern __startup Handle InitializeProcessingUnits();

__extern __startup Handle InitializeModules();

__extern __startup TerminalBase * InitializeTerminalProto();
__extern __startup TerminalBase * InitializeTerminalMain(char * clue);

#ifdef __BEELZEBUB__TEST_MT
__extern __startup void StartMultitaskingTest();
#endif

#ifdef __cplusplus
    #undef Handle
#endif

#undef TerminalBase
