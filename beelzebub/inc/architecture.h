#pragma once

#include <terminals/base.hpp>
#include <metaprogramming.h>

using namespace Beelzebub::Terminals;

__extern void InitializeMemory();
__extern void InitializeInterrupts();

__extern TerminalBase * InitializeTerminalProto();
__extern TerminalBase * InitializeTerminalMain();
