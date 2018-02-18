/*
    Copyright (c) 2018 Alexandru-Mihai Maftei. All rights reserved.


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

#include <djinn.h>
#include "packets.hpp"
#include <string.h>

using namespace Djinn;

//  Initialization

enum DJINN_INIT_STATUS
{
    Uninitialized, InitInProgress, Initialized,
};

::DjinnInitData InitData;
DJINN_INIT_STATUS InitStatus = Uninitialized;
int DebuggerCount = 0;

template<typename TPacket, bool BRetry = true>
static DJINN_SEND_RES Send(TPacket const pack)
{
    DJINN_SEND_RES res;

retry:
    res = InitData.Sender(&pack, sizeof(pack));
    
    if (BRetry && res == DJINN_SEND_AWAIT)
    {
        DJINN_DO_NOTHING();
        goto retry;
    }

    return res;
}

template<typename TPacket, bool BRetry = true>
static DJINN_POLL_RES Poll(TPacket & pack, uint64_t timeout = 0)
{
    DJINN_POLL_RES res;

retry:
    res = InitData.Poller(&pack, sizeof(pack));
    
    if (BRetry && res == DJINN_POLL_AWAIT)
    {
        DJINN_DO_NOTHING();
        goto retry;
    }
    else if (res == DJINN_POLL_NOTHING && timeout > 0)
    {
        --timeout;
        DJINN_DO_NOTHING();
        //  TODO: Some form of proper timing here, e.g. based on the TSC.

        goto retry;
    }

    return res;
}

DJINN_INIT_RES DjinnInitialize(DjinnInitData * data)
{
    DJINN_INIT_STATUS initStatus = Uninitialized;

    if (!__atomic_compare_exchange_n(&InitStatus, &initStatus, InitInProgress, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
        return DJINN_INIT_TOO_LATE;
#define FAIL(val) do { __atomic_store_n(&InitStatus, Uninitialized, __ATOMIC_RELEASE); return (val); } while(false);

    InitData = *data;

    if (InitData.Sender == nullptr || InitData.Poller == nullptr)
        FAIL(DJINN_INIT_DATA_INVALID);

    if (InitData.PacketMaxSize < 6)
        FAIL(DJINN_INIT_PACKET_TOO_SMALL);

    //  Data seems okay. Obtain nuance.

    uint64_t const nuance = 0xF00000FF00FFF000ULL;

    //  Send first handshake packet.

    DJINN_SEND_RES sres = Send(DwordPacket(HandshakePacket0, nuance));

    if (sres != DJINN_SEND_SUCCESS)
        FAIL(DJINN_INIT_HANDSHAKE_FAILED);

    DwordPacket buf1;
    DJINN_POLL_RES pres = Poll(buf1, 10000);
    //  TODO: Timing.

    if (pres == DJINN_POLL_NOTHING)
        goto success;
    //  No response simply means the debugger is not listening.

    if (pres != DJINN_POLL_SUCCESS || buf1.Type != HandshakePacket1)
        FAIL(DJINN_INIT_HANDSHAKE_FAILED);

    sres = Send(DwordPacket(HandshakePacket2, buf1.Payload ^ nuance));

    if (sres != DJINN_SEND_SUCCESS)
        FAIL(DJINN_INIT_HANDSHAKE_FAILED);

    ++DebuggerCount;

#undef FAIL
success:
    __atomic_store_n(&InitStatus, Uninitialized, __ATOMIC_RELEASE);
    return DJINN_INIT_SUCCESS;
}

//  Status

bool DjinnGetInitialized()
{
    return __atomic_load_n(&InitStatus, __ATOMIC_RELAXED) == Initialized;
}

int DjinnGetDebuggerCount()
{
    return __atomic_load_n(&DebuggerCount, __ATOMIC_RELAXED);
}

//  Logging

DjinnLogResult DjinnLog(char const * str, int cnt)
{
    if (__atomic_load_n(&DebuggerCount, __ATOMIC_ACQUIRE) == 0)
        return { DJINN_LOG_NO_DEBUGGERS, 0 };

    if (cnt > (int)(InitData.PacketMaxSize - sizeof(SimplePacket)))
        cnt = (int)(InitData.PacketMaxSize - sizeof(SimplePacket));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvla"
    size_t const pSize = cnt + sizeof(SimplePacket);
    uint8_t buf[pSize];
#pragma GCC diagnostic pop

    new (reinterpret_cast<SimplePacket *>(buf + 0)) SimplePacket(LogPacket);
    memcpy(buf + 2, str, cnt);

    DJINN_SEND_RES res;

retry:
    res = InitData.Sender(buf + 0, pSize);

    if (res == DJINN_SEND_AWAIT)
    {
        DJINN_DO_NOTHING();
        goto retry;
    }

    if (res == DJINN_SEND_SUCCESS)
        return { DJINN_LOG_SUCCESS, cnt };
    else
        return { DJINN_LOG_FAIL, 0 };
}

