/* Modified for standalone mod-llm-commander; compatibility audit 2026-11-03. */
/*
 * This file is part of the LLM bots feature of mod-playerbots (AzerothCore).
 *
 * Deterministic fake provider: returns canned but schema-valid responses with
 * zero network. Used for offline testing of the whole pipeline and as a
 * no-cost fallback. Optionally fails a percentage of calls to exercise the
 * retry / fallback paths.
 */

#ifndef LLMC_MOCK_PROVIDER_H
#define LLMC_MOCK_PROVIDER_H

#include "CommanderLlmProvider.h"

namespace LlmCommander
{

class MockProvider : public LlmProvider
{
public:
    std::string Name() const override;
    bool Configure(ProviderConfig const& config, std::string& err) override;
    LlmResponse Complete(LlmRequest const& request) override;

private:
    uint32_t m_failChancePct = 0;
    uint32_t m_callCount = 0;
};

} // namespace LlmCommander

#endif // MOCK_PROVIDER_H
