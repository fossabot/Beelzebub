#include <system/timers/pit.hpp>
#include <system/io_ports.hpp>
#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::Synchronization;
using namespace Beelzebub::System;
using namespace Beelzebub::System::Timers;

/****************
    Pit class
****************/

/*  Statics  */

Atomic<size_t> Pit::Counter {0};

/*  IRQ Handler  */

void Pit::IrqHandler(INTERRUPT_HANDLER_ARGS)
{
    ++Counter;

    END_OF_INTERRUPT();
}

/*  Initialization  */

void Pit::SetFrequency(uint32_t & freq)
{
    if (freq < MinimumFrequency)
        freq = MinimumFrequency;

    DividerFrequency divfreq = GetRealFrequency(freq);
    freq = divfreq.Frequency;

    Io::Out8(0x40, (uint8_t)(divfreq.Divider     ));  //  Low byte
    Io::Out8(0x40, (uint8_t)(divfreq.Divider >> 8));  //  High byte
}

void Pit::SendCommand(PitCommand const cmd)
{
    Io::Out8(0x43, cmd.Value);
}
