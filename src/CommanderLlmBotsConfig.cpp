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
 */

#include "CommanderLlmBotsConfig.h"

#include "Config.h"
#include "ObjectMgr.h"
#include <cstdlib>

namespace LlmCommander
{
namespace
{
// Alias defaults are registered in .conf.dist so ConfigMgr accepts both key sets.
// A real LlmCommander value overrides its LlmBots counterpart exactly once.
template<class T>
T ReadOption(std::string const& suffix, T const& defaultValue)
{
    std::string const key = "LlmCommander." + suffix;
    if (sConfigMgr->GetOption<std::string>(key, "@legacy@", false) != "@legacy@")
        return sConfigMgr->GetOption<T>(key, defaultValue);
    return sConfigMgr->GetOption<T>("LlmBots." + suffix, defaultValue);
}
}

LlmCommanderConfig LoadLlmCommanderConfig()
{
    static LlmCommanderConfig const cached = []
    {
        LlmCommanderConfig cfg;
        cfg.enable = ReadOption<bool>("Enable", false);
        cfg.provider = ReadOption<std::string>("Provider", "openai");

        cfg.openAiEndpoint = ReadOption<std::string>("Openai.Endpoint", "https://api.openai.com");
        cfg.openAiApiKey = ReadOption<std::string>("Openai.ApiKey", "");
        cfg.modelFast = ReadOption<std::string>("Openai.ModelFast", "");
        cfg.modelThink = ReadOption<std::string>("Openai.ModelThink", "");
        cfg.openAiTimeoutMs = ReadOption<uint32_t>("Openai.TimeoutMs", 30000);

        cfg.foundryEndpoint = ReadOption<std::string>("Foundry.Endpoint", "");
        cfg.foundryApiKey = ReadOption<std::string>("Foundry.ApiKey", "");
        cfg.deploymentFast = ReadOption<std::string>("Foundry.DeploymentFast", "");
        cfg.deploymentThink = ReadOption<std::string>("Foundry.DeploymentThink", "");
        cfg.foundryTimeoutMs = ReadOption<uint32_t>("Foundry.TimeoutMs", 30000);

        cfg.mockFailChance = ReadOption<uint32_t>("Mock.FailChance", 0);

        cfg.workerThreads = ReadOption<uint32_t>("WorkerThreads", 3);
        cfg.maxCallsPerMinute = ReadOption<uint32_t>("MaxCallsPerMinute", 20);
        cfg.maxTokensPerHour = ReadOption<uint32_t>("MaxTokensPerHour", 600000);
        cfg.maxRosterSize = ReadOption<uint32_t>("MaxRosterSize", 20);
        cfg.botAccount = ReadOption<std::string>("BotAccount", "llmbots");

        cfg.commanderName = ReadOption<std::string>("Commander.Name", "CM");
        if (!normalizePlayerName(cfg.commanderName))
            cfg.commanderName.clear();
        cfg.commanderAccountId = ReadOption<uint32_t>("Commander.AccountId", 0);

        cfg.socialCooldownSec = ReadOption<uint32_t>("Social.CooldownSec", 20);
        cfg.planIntervalSec = ReadOption<uint32_t>("Plan.IntervalSec", 300);
        cfg.conversationMaxTurns = ReadOption<uint32_t>("Conversation.MaxTurns", 4);

        cfg.chroniclerName = ReadOption<std::string>("Chronicler.Name", "");
        cfg.chroniclerDigestSec = ReadOption<uint32_t>("Chronicler.DigestSec", 60);

        if (char const* key = std::getenv("LLM_COMMANDER_OPENAI_API_KEY"))
            cfg.openAiApiKey = key;
        if (char const* key = std::getenv("LLM_COMMANDER_FOUNDRY_API_KEY"))
            cfg.foundryApiKey = key;
        return cfg;
    }();
    return cached;
}

} // namespace LlmCommander
