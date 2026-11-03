/* Modified for standalone mod-llm-commander; compatibility audit 2026-11-03. */
/*
 * Modified distribution: mod-playerbots-llm-commander.
 * This file differs from bigr00/mod-llm-playerbots c0ba652.
 * Modified version published on 2026-10-01.
 * Modification notice added on 2026-10-01.
 * Publication date does not identify every historical edit date.
 * Original copyright and license notices remain in effect.
 */
/*
 * This file is part of the LLM bots feature of mod-playerbots (AzerothCore).
 *
 * AzerothCore hooks. Own PlayerScript (game events), WorldScript (world
 * thread pump) and CommandScript (.llmbot) - zero diff to mod-playerbots.
 *
 * Threading rules honoured here:
 *   - Brain ticks and completions run on the world thread after map updates.
 *   - OnWorldUpdate runs on the world thread.
 *   - Chat hooks run on the world thread (session processing).
 *   LLM network I/O is off-thread; roster GM commands use synchronous DB operations.
 */

#include "CommanderLlmBotsModule.h"

#include "Chat.h"
#include "Item.h"
#include "Player.h"
#include "PlayerScript.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "WorldScript.h"

using namespace Acore::ChatCommands;

namespace LlmCommander
{

class LlmCommanderPlayerScript : public PlayerScript
{
public:
    LlmCommanderPlayerScript() : PlayerScript("LlmCommanderPlayerScript") { }

    void OnPlayerLogin(Player* player) override
    {
        sLlmCommanderModule->OnPlayerLogin(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        sLlmCommanderModule->OnPlayerLogout(player);
    }

    // World thread: ordinary say/yell chat is intentionally ignored by LLM bots.
    // Whispers are handled by the receiver-aware overload below.
    bool OnPlayerCanUseChat(Player* /*player*/, uint32_t /*type*/, uint32_t /*language*/, std::string& /*msg*/) override
    {
        return true;
    }

    // World thread: whisper to a roster bot (fires for the whisperer; the
    // receiver pointer is the bot).
    bool OnPlayerCanUseChat(Player* player, uint32_t type, uint32_t /*language*/, std::string& msg, Player* receiver) override
    {
        if (type != CHAT_MSG_WHISPER || !receiver)
        {
            return true;
        }
        sLlmCommanderModule->OnWhisperToRosterBot(player, receiver, msg);
        return true;
    }

    void OnPlayerJustDied(Player* player) override
    {
        sLlmCommanderModule->OnBotJustDied(player);
    }

    void OnPlayerLevelChanged(Player* player, uint8_t /*oldlevel*/) override
    {
        sLlmCommanderModule->OnBotLevelUp(player);
    }

    void OnPlayerLootItem(Player* player, Item* item, uint32_t /*count*/, ObjectGuid /*lootguid*/) override
    {
        sLlmCommanderModule->OnBotLoot(player, item);
    }

    void OnPlayerDuelRequest(Player* target, Player* challenger) override
    {
        sLlmCommanderModule->OnDuelRequest(target, challenger);
    }
};

class LlmCommanderWorldScript : public WorldScript
{
public:
    LlmCommanderWorldScript() : WorldScript("LlmCommanderWorldScript") { }

    void OnStartup() override
    {
        sLlmCommanderModule->Initialize();
    }

    void OnShutdown() override
    {
        sLlmCommanderModule->Shutdown();
    }

    void OnUpdate(uint32_t diff) override
    {
        sLlmCommanderModule->OnWorldTick(diff);
    }
};

class LlmCommanderCommandScript : public CommandScript
{
public:
    LlmCommanderCommandScript() : CommandScript("LlmCommanderCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable llmbotCommands =
        {
            { "list",    HandleLlmBotList,    rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "add",     HandleLlmBotAdd,     rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "remove",  HandleLlmBotRemove,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "persona", HandleLlmBotPersona, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "reload",  HandleLlmBotReload,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "stats",   HandleLlmBotStats,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "say",     HandleLlmBotSay,     rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "goal",    HandleLlmBotGoal,    rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "pause",   HandleLlmBotPause,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "resume",  HandleLlmBotResume,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "llmbot", llmbotCommands }
        };

        return commandTable;
    }

    static bool HandleLlmBotList(ChatHandler* handler)
    {
        std::string out = "Roster:";
        for (auto const& entry : sLlmCommanderModule->Registry().Entries(false))
        {
            out += " " + entry.name;
            if (!entry.enabled)
            {
                out += "(disabled)";
            }
        }
        handler->SendSysMessage(out.c_str());
        return true;
    }

    static bool HandleLlmBotAdd(ChatHandler* handler, std::string const& name, Optional<std::string> persona)
    {
        if (!handler->GetSession())
        {
            return false;
        }
        sLlmCommanderModule->CmdAdd(handler->GetSession()->GetPlayer(), name, persona.value_or(""));
        return true;
    }

    static bool HandleLlmBotRemove(ChatHandler* handler, std::string const& name)
    {
        if (!handler->GetSession())
        {
            return false;
        }
        sLlmCommanderModule->CmdRemove(handler->GetSession()->GetPlayer(), name);
        return true;
    }

    static bool HandleLlmBotPersona(ChatHandler* handler, std::string const& name, std::string const& persona)
    {
        if (!handler->GetSession())
        {
            return false;
        }
        sLlmCommanderModule->CmdPersona(handler->GetSession()->GetPlayer(), name, persona);
        return true;
    }

    static bool HandleLlmBotReload(ChatHandler* handler)
    {
        sLlmCommanderModule->ReloadRoster();
        handler->SendSysMessage("Roster reload requested");
        return true;
    }

    static bool HandleLlmBotStats(ChatHandler* handler)
    {
        if (!handler->GetSession())
        {
            return false;
        }
        sLlmCommanderModule->CmdStats(handler->GetSession()->GetPlayer());
        return true;
    }

    static bool HandleLlmBotSay(ChatHandler* handler, std::string const& name, std::vector<std::string> promptWords)
    {
        if (!handler->GetSession())
        {
            return false;
        }
        std::string prompt;
        for (auto const& word : promptWords)
        {
            if (!prompt.empty())
            {
                prompt += " ";
            }
            prompt += word;
        }
        sLlmCommanderModule->CmdSay(handler->GetSession()->GetPlayer(), name, prompt);
        return true;
    }

    static bool HandleLlmBotGoal(ChatHandler* handler, std::string const& name)
    {
        if (!handler->GetSession())
        {
            return false;
        }
        sLlmCommanderModule->CmdGoal(handler->GetSession()->GetPlayer(), name);
        return true;
    }

    static bool HandleLlmBotPause(ChatHandler* handler)
    {
        if (handler->GetSession())
        {
            sLlmCommanderModule->CmdPause(handler->GetSession()->GetPlayer(), true);
        }
        return true;
    }

    static bool HandleLlmBotResume(ChatHandler* handler)
    {
        if (handler->GetSession())
        {
            sLlmCommanderModule->CmdPause(handler->GetSession()->GetPlayer(), false);
        }
        return true;
    }
};

} // namespace LlmCommander

void Addmod_llm_commanderScripts()
{
    using namespace LlmCommander;
    new LlmCommanderPlayerScript();
    new LlmCommanderWorldScript();
    new LlmCommanderCommandScript();
}
