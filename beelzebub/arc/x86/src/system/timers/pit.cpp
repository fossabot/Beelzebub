#include <system/timers/pit.hpp>
#include <system/lapic.hpp>
#include <system/io_ports.hpp>
#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::System;
using namespace Beelzebub::System::Timers;

/****************
    Pit class
****************/

/*  IRQ Handler  */

void Pit::IrqHandler(INTERRUPT_HANDLER_ARGS)
{
    //msg("PIT!");

    END_OF_INTERRUPT();
}

/*  Initialization  */

void Pit::SetFrequency(uint32_t & freq)
{
    DividerFrequency divfreq = GetRealFrequency(freq);
    freq = divfreq.Frequency;

    Io::Out8(0x40, (uint8_t)(divfreq.Divider     ));  //  Low byte
    Io::Out8(0x40, (uint8_t)(divfreq.Divider >> 8));  //  High byte
}

void Pit::SendCommand(PitCommand const cmd)
{
    Io::Out8(0x43, cmd.Value);
}
