/* Modified for standalone mod-llm-commander; compatibility audit 2026-11-03. */
/*
 * This file is part of the LLM bots feature of mod-playerbots (AzerothCore).
 *
 * Budget and call-rate governor. Thread-safe: Submit() and Record() may be
 * called from any thread. A call must pass the governor to be enqueued.
 */

#ifndef LLMC_LLM_BUDGET_H
#define LLMC_LLM_BUDGET_H

#include "CommanderLlmTypes.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>

namespace LlmCommander
{

enum class BudgetResult : uint8_t
{
    Allowed,
    CallRateExceeded, // calls-per-minute bucket empty
    TokenBudgetExceeded,
    Degraded,         // at 80%: only Tier 1 (social) calls allowed
    Disabled
};

struct BudgetStats
{
    uint64_t calls = 0;
    uint64_t refused = 0;
    uint64_t errors = 0;
    uint64_t tokensPrompt = 0;
    uint64_t tokensCompletion = 0;
    uint64_t latencyTotalMs = 0;
    uint64_t latencyCount = 0;
    uint64_t fallbacks = 0; // degraded to canned text
    uint64_t byPurpose[8] = { 0 }; // indexed by Purpose
};

class LlmBudget
{
public:
    void Configure(uint32_t maxCallsPerMinute, uint32_t maxTokensPerHour, uint32_t maxQueueDepth);

    // Called before enqueue. Never blocks.
    BudgetResult Check(Purpose purpose, ModelTier tier,
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

    // Called when a request enters the worker queue.
    void OnQueued();
    void OnDequeued();

    // Called when a response (or error) comes back.
    void Record(Purpose purpose, ModelTier tier, LlmResponse const& response, bool fallbackUsed);

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    BudgetStats GetStats() const;

    // Rolling hourly token total for display/logs.
    uint32_t TokensThisHour() const;

private:
    mutable std::mutex m_mutex;
    std::chrono::steady_clock::time_point m_hourStart = std::chrono::steady_clock::now();
    uint32_t m_tokensThisHour = 0;

    uint32_t m_maxCallsPerMinute = 0;
    uint32_t m_maxTokensPerHour = 0;
    uint32_t m_maxQueueDepth = 0;
    double m_bucketTokens = 0;
    std::chrono::steady_clock::time_point m_bucketRefill = std::chrono::steady_clock::now();

    std::atomic<uint32_t> m_queued = 0;
    std::atomic<bool> m_enabled = false;

    BudgetStats m_stats;
};

} // namespace LlmCommander

#endif // LLM_BUDGET_H
