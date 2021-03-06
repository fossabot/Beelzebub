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

#pragma once

#ifdef __BEELZEBUB
    #include <beel/metaprogramming.h>

    #define DJINN_DO_NOTHING DO_NOTHING

    #if !defined(__BEELZEBUB__PLACEMENT_NEW) && defined(_DJINN) && defined(__cplusplus)
        #define __BEELZEBUB__PLACEMENT_NEW
        
        inline void * operator new  (std::size_t, void * p) noexcept { return p; }
        inline void * operator new[](std::size_t, void * p) noexcept { return p; }
        inline void operator delete  (void *, void *) noexcept { }
        inline void operator delete[](void *, void *) noexcept { }
    #endif

    #define DJINN_FUNC __noinline __noclone
#elif defined(_ALADIN)
    #ifdef __cplusplus
        #include <cstdint>
    #else
        #include <stdint.h>
    #endif
#else
    #error "Unknown host; Djinn needs some architecture info at least."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ALADIN

//  Sending

enum DJINN_SEND_RES
{
    DJINN_SEND_SUCCESS,
    DJINN_SEND_AWAIT,   //  Medium bandwidth constraint.
    DJINN_SEND_PACKET_SIZE_OOR, //  Packet outside of size range given at init; this causes system halt.
};

/*  PRECONDITION: IRQs disabled  */
typedef enum DJINN_SEND_RES (*DjinnPacketSender)(void const * packet, size_t size);

//  Polling (aka synchronous receiving)

enum DJINN_POLL_RES
{
    DJINN_POLL_SUCCESS,
    DJINN_POLL_NOTHING, //  No packet incomming.
    DJINN_POLL_AWAIT,   //  Medium bandwidth constraint.
    DJINN_POLL_PACKET_SIZE_OOR, //  Packet outside of size range given at init; this causes system halt.
};

/*  PRECONDITION: IRQs disabled  */
typedef enum DJINN_POLL_RES (*DjinnPacketPoller)(void * buffer, size_t capacity, size_t * len);

//  Processing (aka asynchronous receiving)

enum DJINN_PROC_RES
{
    DJINN_PROC_SUCCESS,
    DJINN_PROC_PACKET_SIZE_OOR,
    DJINN_PROC_PACKET_MALFORMED,
};

/*  PRECONDITION: IRQs disabled  */
enum DJINN_PROC_RES DjinnProcessPacket(void const * packet, size_t size);

//  Initialization

struct DjinnInitData
{
    DjinnPacketSender Sender;
    DjinnPacketPoller Poller;
    size_t PacketMaxSize;
};

enum DJINN_INIT_RES
{
    DJINN_INIT_SUCCESS,
    DJINN_INIT_TOO_LATE,
    DJINN_INIT_DATA_INVALID,
    DJINN_INIT_PACKET_TOO_SMALL,
    DJINN_INIT_HANDSHAKE_FAILED,
};

/*  PRECONDITION: IRQs disabled  */
enum DJINN_INIT_RES DjinnInitialize(struct DjinnInitData * data);

//  Status

DJINN_FUNC bool DjinnGetInitialized();
DJINN_FUNC int DjinnGetDebuggerCount();

//  Logging

enum DJINN_LOG_RES
{
    DJINN_LOG_SUCCESS,
    DJINN_LOG_NO_DEBUGGERS,
    DJINN_LOG_FAIL,
};

struct DjinnLogResult
{
    enum DJINN_LOG_RES Result;
    int Count;
};

DJINN_FUNC struct DjinnLogResult DjinnLog(char const * str, int cnt);

enum DJINN_LOG_INT_FORMAT
{
    DJINN_INT_DEC,
    DJINN_INT_UDEC,
    DJINN_INT_HEX8,
    DJINN_INT_HEX16,
    DJINN_INT_HEX24,
    DJINN_INT_HEX32,
    DJINN_INT_HEX48,
    DJINN_INT_HEX64,
    DJINN_INT_HEX_VAR,
};

DJINN_FUNC struct DjinnLogResult DjinnLogUInt(uint64_t val, enum DJINN_LOG_INT_FORMAT fmt);

static struct DjinnLogResult DjinnLogInt(int64_t val, enum DJINN_LOG_INT_FORMAT fmt)
{
    return DjinnLogUInt((uint64_t)val, fmt);
}

#endif  //  defined(_ALADIN)

#ifndef __cplusplus

//  Packets

#define DJINN_PACKET_INVALID            ((uint16_t)0x0000)
#define DJINN_PACKET_HANDSHAKE0         ((uint16_t)0x0001)
#define DJINN_PACKET_HANDSHAKE1         ((uint16_t)0x0002)
#define DJINN_PACKET_HANDSHAKE2         ((uint16_t)0x0003)

#define DJINN_PACKET_LOG                ((uint16_t)0x0010)
#define DJINN_PACKET_LOG_INT_DEC        ((uint16_t)0x0011)
#define DJINN_PACKET_LOG_INT_UDEC       ((uint16_t)0x0012)
#define DJINN_PACKET_LOG_INT_HEX8       ((uint16_t)0x0013)
#define DJINN_PACKET_LOG_INT_HEX16      ((uint16_t)0x0014)
#define DJINN_PACKET_LOG_INT_HEX24      ((uint16_t)0x0015)
#define DJINN_PACKET_LOG_INT_HEX32      ((uint16_t)0x0016)
#define DJINN_PACKET_LOG_INT_HEX48      ((uint16_t)0x0017)
#define DJINN_PACKET_LOG_INT_HEX64      ((uint16_t)0x0018)
#define DJINN_PACKET_LOG_INT_HEX_VAR    ((uint16_t)0x0019)

#pragma pack(1)

struct DjinnSimplePacket
{
    uint16_t Type;
};

struct DjinnDwordPacket
{
    uint16_t Type;
    uint64_t Payload;
};

struct DjinnGenericPacket
{
    uint16_t Type;
    uint8_t Payload[];
};

#pragma pack()

#else   //  #ifndef __cplusplus
}   //  extern "C"

namespace Djinn
{

//  Packets

enum PacketType : uint16_t
{
    InvalidPacket       = 0x0000,

    HandshakePacket0    = 0x0001,
    HandshakePacket1    = 0x0002,
    HandshakePacket2    = 0x0003,

    LogPacket           = 0x0010,
    LogIntDecPacket     = 0x0011,
    LogIntUDecPacket    = 0x0012,
    LogIntHex8Packet    = 0x0013,
    LogIntHex16Packet   = 0x0014,
    LogIntHex24Packet   = 0x0015,
    LogIntHex32Packet   = 0x0016,
    LogIntHex48Packet   = 0x0017,
    LogIntHex64Packet   = 0x0018,
    LogIntHexVarPacket  = 0x0019,
};

#pragma pack(1)

struct SimplePacket
{
    PacketType Type;

    inline SimplePacket() : Type(InvalidPacket) { }
    inline explicit SimplePacket(PacketType type) : Type(type) { }
};

struct DwordPacket : SimplePacket
{
    uint64_t Payload;

    inline DwordPacket() : SimplePacket(), Payload(0) { }
    inline explicit DwordPacket(PacketType type, uint64_t payload) : SimplePacket(type), Payload(payload) { }
};

struct GenericPacket : SimplePacket
{
    uint8_t Payload[];
};

#pragma pack()

}  //   namespace Djinn
#endif  //  #ifndef __cplusplus
