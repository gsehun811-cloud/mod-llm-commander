/* Modified for standalone mod-llm-commander; compatibility audit 2026-11-03. */
/*
 * This file is part of the LLM bots feature of mod-playerbots (AzerothCore).
 */

#include "CommanderLlmProvider.h"

#include "CommanderFoundryProvider.h"
#include "CommanderMockProvider.h"
#include "CommanderOpenAiProvider.h"

namespace LlmCommander
{

LlmProvider* CreateProvider(std::string const& name)
{
    if (name == "openai")
    {
        return new OpenAiProvider();
    }
    if (name == "foundry")
    {
        return new FoundryProvider();
    }
    if (name == "mock")
    {
        return new MockProvider();
    }
    return nullptr;
}

} // namespace LlmCommander
