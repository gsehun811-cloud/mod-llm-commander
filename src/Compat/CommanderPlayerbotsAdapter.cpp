/* GPL-2.0-or-later. No Playerbots implementation is copied into this adapter. */
#include "CommanderPlayerbotsAdapter.h"
#include "CommanderPresetPolicy.h"
#include "Script/WorldThr/PlayerbotOperations.h"
#include "Script/WorldThr/PlayerbotWorldThreadProcessor.h"

#include <memory>

namespace LlmCommander::Compat
{
static_assert(requires(ObjectGuid guid, std::unique_ptr<PlayerbotOperation> operation)
{
    AddPlayerBotOperation(guid, uint32_t{0});
    { PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(operation)) } -> std::convertible_to<bool>;
    { sRandomPlayerbotMgr.GetPlayerBot(guid) } -> std::convertible_to<Player*>;
    sRandomPlayerbotMgr.LogoutPlayerBot(guid);
}, "Unsupported Playerbots roster login/logout interface; extend the Commander adapter.");

PlayerbotAI* GetAI(Player* player)
{
#if LLMC_LOOKUP_MANAGER
    return player ? sPlayerbotsMgr.GetPlayerbotAI(player) : nullptr;
#else
    return player ? GET_PLAYERBOT_AI(player) : nullptr;
#endif
}

bool IsRandomBot(Player* player)
{
    return player && sRandomPlayerbotMgr.IsRandomBot(player);
}

Player* FindBot(ObjectGuid guid)
{
    return sRandomPlayerbotMgr.GetPlayerBot(guid);
}

void QueueLogin(ObjectGuid guid)
{
    PlayerbotWorldThreadProcessor::instance().QueueOperation(
        std::make_unique<AddPlayerBotOperation>(guid, /*masterAccountId=*/0));
}

void Logout(ObjectGuid guid)
{
    sRandomPlayerbotMgr.LogoutPlayerBot(guid);
}

bool HasTalentPreset(uint8_t classId, std::string const& preset)
{
    if (classId >= MAX_CLASSES)
        return false;
    return ContainsNativePreset(sPlayerbotAIConfig.PremadeSpecName[classId], preset);
}

bool Dispatch(PlayerbotAI* ai, std::string const& command, Player* issuer)
{
    if (!ai || !issuer || !ai->GetAiObjectContext())
        return false;
    ExternalEventHelper helper(ai->GetAiObjectContext());
    return helper.ParseChatCommand(command, issuer);
}
}
