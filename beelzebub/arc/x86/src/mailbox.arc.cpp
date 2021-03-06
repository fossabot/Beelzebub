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

/*
Gotta give credit to `geist` over at #osdev@Freenode for sharing his ideas and
experience related to the mailbox. I don't think I could've come up with such a
lean & mean implementation without his aid.
 */

#ifdef __BEELZEBUB_SETTINGS_SMP

#include "mailbox.hpp"
#include "cores.hpp"
#include "system/interrupt_controllers/lapic.hpp"
#include "system/nmi.hpp"
#include "kernel.hpp"
#include "irqs.hpp"
#include <beel/sync/smp.lock.hpp>
#include <string.h>

using namespace Beelzebub;
using namespace Beelzebub::Synchronization;
using namespace Beelzebub::System;
using namespace Beelzebub::System::InterruptControllers;

/****************
    Internals
****************/

#ifdef __BEELZEBUB_SETTINGS_MANYCORE
static SmpLock GlobalLock {};
static MailboxEntryBase * volatile GlobalHead = nullptr;
static MailboxEntryBase * GlobalTail = nullptr;
static size_t GlobalGeneration = 0;
#endif

static __hot void ExecuteNmMail(InterruptContext const * context, void * isrCookie)
{
    (void)context;
    (void)isrCookie;

    CpuData * const data = Cpu::GetData();

    MailboxEntryBase * entry;

    while ((entry = data->MailNmTop.Xchg(nullptr)) != nullptr)
        do
        {
            MailFunction const func = entry->Function;
            void * const cookie = entry->Cookie;
            Synchronization::Atomic<unsigned int> * dstCtr = nullptr;

            for (unsigned int i = 0; i < entry->DestinationCount; ++i)
            {
                MailboxEntryLink & link = entry->Links[i];

                if (link.Core == data->Index)
                {
                    if (entry->GetAwait())
                        dstCtr = &(entry->DestinationsLeft);
                    else
                    {
                        --entry->DestinationsLeft;
                        //  This core no longer needs anything from that mail entry.
                    }

                    entry = link.Next;
                    //  Prepares for the next entry as well.

                    break;
                }
            }

            func(cookie);

            if unlikely(dstCtr != nullptr)
                dstCtr->operator --();
        } while (entry != nullptr);
}

static __hot __solid bool ExecuteHead()
{
    CpuData * const data = Cpu::GetData();

    MailFunction func = nullptr;
    void * cookie = nullptr;
    Synchronization::Atomic<unsigned int> * dstCtr = nullptr;

    withLock (data->MailLock)
    {
        auto head = data->MailHead;

        if unlikely(head == nullptr)
#ifdef __BEELZEBUB_SETTINGS_MANYCORE
            goto check_global;
#else
            return false;
#endif

        func = head->Function;
        cookie = head->Cookie;

        for (unsigned int i = 0; i < head->DestinationCount; ++i)
        {
            MailboxEntryLink & link = head->Links[i];

            if (link.Core == data->Index)
            {
                data->MailHead = link.Next;
                //  This dequeues the mail entry.

                if (head->GetAwait())
                    dstCtr = &(head->DestinationsLeft);
                else
                {
                    --head->DestinationsLeft;
                    //  This core no longer needs anything from that mail entry.
                }

                break;
            }
        }

        if (data->MailHead == nullptr)
            data->MailTail = nullptr;
    }

#ifdef __BEELZEBUB_SETTINGS_MANYCORE
    goto execute;

check_global:
    withLock (GlobalLock)
    {
        MailboxEntryBase * entry = GlobalHead;

        while (entry != nullptr)
        {
            if (entry->Links[0].Generation > data->MailGeneration)
            {
                //  Found one!

                data->MailGeneration = entry->Links[0].Generation;
                //  This becomes the last generation.

                func = entry->Function;
                cookie = entry->Cookie;

                --entry->DestinationsLeft;

                break;
            }

            entry = entry->Links[0].Next;
        }

        if (entry == nullptr)
            return false;
    }

execute:
#endif

    func(cookie);

    if unlikely(dstCtr != nullptr)
        dstCtr->operator --();

    return true;
}

static __hot __realign_stack void MailboxIsrHandler(InterruptContext const * context, void * cookie)
{
    (void)context;
    (void)cookie;

    while (ExecuteHead()) { /* loopie loop */ }
}

static __hot void PostInternal(MailboxEntryBase * entry, TimeWaster waster, void * cookie, bool poll, bool broadcast)
{
    for (unsigned int i = 0; i < entry->DestinationCount; ++i)
    {
        uint32_t const targetCore = entry->Links[i].Core;
        CpuData * const target = Cores::Get(targetCore);

        if (entry->GetNonMaskable())
        {
            MailboxEntryBase * top = target->MailNmTop.Load();

            do entry->Links[i].Next = top; while (!target->MailNmTop.CmpXchgStrong(top, entry));
        }
        else
        {
            withLock (target->MailLock)
            {
                if likely(target->MailTail != nullptr)
                {
                    //  Mail is already queued for this core, so the entry is added to the tail.

                    MailboxEntryBase * const tail = target->MailTail;

                    for (unsigned int j = 0; j < entry->DestinationCount; ++j)
                    {
                        MailboxEntryLink & link = tail->Links[j];

                        if (link.Core == targetCore)
                        {
                            link.Next = entry;

                            break;
                        }
                    }

                    target->MailTail = entry;
                }
                else
                    target->MailTail = target->MailHead = entry;    //  ezpz
            }
        }

        if unlikely(!broadcast)
        {
            if (entry->GetNonMaskable())
                Nmi::Send(target->LapicId);
            else
                Lapic::SendIpi(LapicIcr(0)
                    .SetDeliveryMode(InterruptDeliveryModes::Fixed)
                    .SetDestinationShorthand(IcrDestinationShorthand::None)
                    .SetAssert(true)
                    .SetDestination(target->LapicId)
                    .SetVector(KnownIsrs::Mailbox));
        }
    }

    if likely(broadcast)
    {
        if (entry->GetNonMaskable())
            Nmi::Broadcast();
        else
            Lapic::SendIpi(LapicIcr(0)
                .SetDeliveryMode(InterruptDeliveryModes::Fixed)
                .SetDestinationShorthand(IcrDestinationShorthand::AllExcludingSelf)
                .SetAssert(true)
                .SetVector(KnownIsrs::Mailbox));
    }

    if (waster != nullptr)
        waster(cookie);

    unsigned int destLeft;

    while ((destLeft = entry->DestinationsLeft) > 0)
        if (!(poll && ExecuteHead()))
            do CpuInstructions::DoNothing(); while (--destLeft > 0);
}

#ifdef __BEELZEBUB_SETTINGS_MANYCORE
static __hot void PostGlobal(MailboxEntryBase * entry, TimeWaster waster, void * cookie, bool poll)
{
    withLock (GlobalLock)
    {
        auto gen = ++GlobalGeneration;
        //  This be the generation of the mail entry.

        entry->Links[0].Generation = gen;

        if (GlobalTail == nullptr)
            GlobalHead = GlobalTail = entry;
        else
        {
            GlobalTail->Links[0].Next = entry;

            assert(GlobalTail->Links[0].Generation < gen)
                (GlobalTail->Links[0].Generation)(gen);

            GlobalTail = entry;
        }
    }

    Lapic::SendIpi(LapicIcr(0)
        .SetDeliveryMode(InterruptDeliveryModes::Fixed)
        .SetDestinationShorthand(IcrDestinationShorthand::AllExcludingSelf)
        .SetAssert(true)
        .SetVector(KnownIsrs::Mailbox);

    if (waster != nullptr)
        waster(cookie);

    {   //  Scope to contain the variable.
        unsigned int destLeft;

        while ((destLeft = entry->DestinationsLeft) > 0)
            if (!(poll && ExecuteHead()))
                do CpuInstructions::DoNothing(); while (--destLeft > 0);
    }

    //  So, the mail had been handled by all destinations.
    //  Now it needs to become the head of the queue to be removed from it.

    while (entry != GlobalHead)
        if (!(poll && ExecuteHead()))
            CpuInstructions::DoNothing();

    //  Now it should be the head of the queue.

    withLock (GlobalLock)
    {
        assert(entry == GlobalHead)((void *)entry)((void *)GlobalHead);
        //  If another core dequeued it, it's a huge problem.

        entry = entry->Links[0].Next;

        if (entry == nullptr)
            GlobalHead = GlobalTail = nullptr;
        else
            GlobalHead = entry;
    }
}
#endif

static SmpLock InitLock {};
static bool Initialized = false;
static Atomic<size_t> InitializedCount {0};
static bool FullyInitialized = false;

/********************
    Mailbox class
********************/

static InterruptHandlerNode StandardHandler { &MailboxIsrHandler };
static InterruptHandlerNode NmiHandler { &ExecuteNmMail };

/*  Initialization  */

void Mailbox::Initialize()
{
    //  A lock is used here because this code must only be executed once, and
    //  other cores should wait for it to finish.

    withLock (InitLock)
    {
        if likely(!Initialized)
        {
            ASSERT(StandardHandler.Subscribe(Irqs::Mailbox) == IrqSubscribeResult::Success);
            ASSERT(Lapic::Ender.Register(Irqs::Mailbox) == IrqEnderRegisterResult::Success);

            ASSERT(NmiHandler.Subscribe(Irqs::Nmi) == IrqSubscribeResult::Success);

            Initialized = true;
        }
    }

    if (++InitializedCount == Cores::GetCount())
        FullyInitialized = true;
}

bool Mailbox::IsReady()
{
    return FullyInitialized;
}

/*  Operation  */

void Mailbox::Post(MailboxEntryBase * entry, TimeWaster waster, void * cookie, bool poll)
{
    assert(entry != nullptr);

    InterruptGuard<> intGuard;
    //  Everything *has* to be done under a lock guard.

    if (entry->DestinationCount == 1 && entry->Links[0].Core == Broadcast)
    {
        auto tgCnt = Cores::GetCount() - 1;     //  Target count.

#ifdef __BEELZEBUB_SETTINGS_MANYCORE
        if likely(tgCnt < 64)
        {
#endif
            auto thisCore = Cpu::GetData()->Index;  //  Index of this core.

            ALLOCATE_MAIL(newEntry, tgCnt, entry->Function, entry->Cookie);
            newEntry.Flags = entry->Flags;

            for (unsigned int link = 0; link < tgCnt; ++link)
                newEntry.Links[link] = MailboxEntryLink((link < thisCore) ? link : (link + 1));
            //  Set up all the links properly.

            return PostInternal(&newEntry, waster, cookie, poll, true);
#ifdef __BEELZEBUB_SETTINGS_MANYCORE
        }
        else
        {
            entry->DestinationsLeft = tgCnt;

            return PostGlobal(entry, waster, cookie, poll);
        }
#endif
    }
    else
        return PostInternal(entry, waster, cookie, poll, false);
}

#endif
