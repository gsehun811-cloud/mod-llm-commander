/* GPL-2.0-or-later. Shared, deny-by-default action policy. */
#pragma once
#include <cstdint>

namespace LlmCommander
{
constexpr bool CanExecuteAction(bool isCommander, bool actorHasAI, bool issuerIsHuman,
    uint32_t authorizedAccount, uint32_t issuerAccount)
{
    return isCommander && actorHasAI && issuerIsHuman && authorizedAccount != 0 &&
        authorizedAccount == issuerAccount;
}
}
