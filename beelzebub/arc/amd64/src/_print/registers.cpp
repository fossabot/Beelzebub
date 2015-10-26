#include <_print/registers.hpp>
#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::Debug;
using namespace Beelzebub::System;
using namespace Beelzebub::Terminals;

/*****************
    Cr0 Struct
*****************/

TerminalWriteResult PrintToTerminal(TerminalBase * const term, Cr0 const val)
{
    char str[52] = "{CR0|PME0|MC0|Em0|TS0|ET0|NE0|WP0|AM0|NWT0|CD0|Pg0}";

    if (val.GetProtectedModeEnable())
        str[ 8] = '1';
    if (val.GetMonitorCoprocessor())
        str[12] = '1';
    if (val.GetEmulation())
        str[16] = '1';
    if (val.GetTaskSwitched())
        str[20] = '1';
    if (val.GetExtensionType())
        str[24] = '1';
    if (val.GetNumericError())
        str[28] = '1';
    if (val.GetWriteProtect())
        str[32] = '1';
    if (val.GetAlignmentMask())
        str[36] = '1';
    if (val.GetNotWriteThrough())
        str[41] = '1';
    if (val.GetCacheDisable())
        str[45] = '1';
    if (val.GetPaging())
        str[49] = '1';

    return term->Write(str);
}

TerminalWriteResult PrintToDebugTerminal(Cr0 const val)
{
    return PrintToTerminal(DebugTerminal, val);
}

/*****************
    Cr3 Struct
*****************/

TerminalWriteResult PrintToTerminal(TerminalBase * const term, Cr3 const val)
{
    char str[37] = "{CR3|                |   |PWT0|PCD0}";

    uint64_t adr = (uint64_t)val.GetAddress();

    for (size_t i = 0; i < 16; ++i)
    {
        uint8_t nib = (adr >> (i * 4)) & 0xF;

        str[20 - i] = (nib > 9 ? '7' : '0') + nib;
    }

    uint64_t pid = (uint64_t)val.GetPcid();

    for (size_t i = 0; i < 3; ++i)
    {
        uint8_t nib = (pid >> (i * 4)) & 0xF;

        str[24 - i] = (nib > 9 ? '7' : '0') + nib;
    }

    if (val.GetPwt())
        str[29] = '1';
    if (val.GetPcd())
        str[34] = '1';

    return term->Write(str);
}

TerminalWriteResult PrintToDebugTerminal(Cr3 const val)
{
    return PrintToTerminal(DebugTerminal, val);
}
