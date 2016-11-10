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

#include <system/interrupts.hpp>
#include <system/cpu.hpp>

#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::System;

__extern void const * InterruptHandlers[256];
__extern InterruptEnderFunction InterruptEnders[256];

__extern void IsrCommonStub(void);
__extern void IsrFullStub(void);

/***********************
    Interrupts class
***********************/

/*  Statics  */

Idt Interrupts::Table;
IdtRegister Interrupts::Register {0xFFF, &Interrupts::Table};

/*****************************
    Interrupts::Data class
*****************************/

/*  Handler & Ender  */

void const * Interrupts::Data::GetHandler() const
{
    return InterruptHandlers[this->Vector];
}

InterruptEnderFunction Interrupts::Data::GetEnder() const
{
    return InterruptEnders[this->Vector];
}

Interrupts::Data const & Interrupts::Data::SetHandler(InterruptHandlerPartialFunction const val) const
{
    InterruptHandlers[this->Vector] = reinterpret_cast<void const *>(val);

    auto stub = this->GetStub();

    if (stub->GetJumpTarget() == &IsrFullStub)
    {
        withWriteProtect (false)
            stub->SetJumpTarget(reinterpret_cast<void const *>(&IsrCommonStub));

        CpuInstructions::FlushCache(&IsrStubsBegin + this->Vector);
    }

    return *this;
}

Interrupts::Data const & Interrupts::Data::SetHandler(InterruptHandlerFullFunction const val) const
{
    InterruptHandlers[this->Vector] = reinterpret_cast<void const *>(val);

    auto stub = this->GetStub();

    if (stub->GetJumpTarget() == &IsrCommonStub)
    {
        withWriteProtect (false)
            stub->SetJumpTarget(reinterpret_cast<void const *>(&IsrFullStub));

        CpuInstructions::FlushCache(&IsrStubsBegin + this->Vector);
    }

    return *this;
}

Interrupts::Data const & Interrupts::Data::RemoveHandler() const
{
    return this->SetHandler(reinterpret_cast<InterruptHandlerPartialFunction>((void *)(nullptr)));
}

Interrupts::Data const & Interrupts::Data::SetEnder(InterruptEnderFunction const val) const
{
    InterruptEnders[this->Vector] = val;

    return *this;
}

/*  Properties  */

bool Interrupts::Data::IsFull() const
{
    return this->GetStub()->GetJumpTarget() == &IsrFullStub;
}
