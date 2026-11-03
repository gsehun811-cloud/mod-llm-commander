/* GPL-2.0-or-later. External Playerbots compatibility boundary. */
#pragma once

#include "CommanderCompatibility.h"
#include "Playerbots.h"
#include "ExternalEventHelper.h"
#include "Event.h"

#include <concepts>
#include <cstdint>

namespace LlmCommander::Compat
{
static_assert(requires(PlayerbotAI& ai, Player* player, std::string command, Event event)
{
    { ai.GetMaster() } -> std::convertible_to<Player*>;
    { ai.IsAltBot() } -> std::convertible_to<bool>;
    { ai.DoSpecificAction(command, event, true) } -> std::convertible_to<bool>;
    { ai.GetAiObjectContext() };
    { ai.Say(command) };
    { ai.Whisper(command, command) };
}, "Unsupported Playerbots interface. Implement a new Commander adapter for this version.");

static_assert(requires(PlayerbotAIConfig const& config)
{
    config.PremadeSpecName[0][0];
}, "Unsupported native talent preset storage; extend the Commander compatibility adapter.");

PlayerbotAI* GetAI(Player* player);
bool HasTalentPreset(uint8_t classId, std::string const& preset);
bool IsRandomBot(Player* player);
Player* FindBot(ObjectGuid guid);
void QueueLogin(ObjectGuid guid);
void Logout(ObjectGuid guid);
bool Dispatch(PlayerbotAI* ai, std::string const& command, Player* issuer);
}
