#include "CommanderAuthorization.h"
#include "CommanderPresetPolicy.h"
#include "CommanderHttpsClient.h"
#include "CommanderLlmBudget.h"
#include "CommanderLlmSchema.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace LlmCommander;

void Require(bool condition, char const* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

int main()
{
    try
    {
        // Each missing identity/authorization condition independently denies actions.
        Require(CanExecuteAction(true, true, true, 17, 17), "authorized commander denied");
        Require(!CanExecuteAction(false, true, true, 17, 17), "ordinary companion gained actions");
        Require(!CanExecuteAction(true, false, true, 17, 17), "human actor gained actions");
        Require(!CanExecuteAction(true, true, false, 17, 17), "bot issuer gained actions");
        Require(!CanExecuteAction(true, true, true, 0, 0), "unconfigured account gained actions");
        Require(!CanExecuteAction(true, true, true, 17, 18), "foreign account gained actions");

        std::string const presets[] = { "arms pve", "prot pve", "" };
        Require(Compat::ContainsNativePreset(presets, "prot pve"), "configured preset rejected");
        Require(!Compat::ContainsNativePreset(presets, "holy pve"), "missing preset accepted");
        Require(!Compat::ContainsNativePreset(presets, "Prot PvE"),
            "native case-sensitive lookup changed");
        Require(!Compat::ContainsNativePreset(presets, ""), "empty preset accepted");
        std::string const interrupted[] = { "arms pve", "", "prot pve" };
        Require(!Compat::ContainsNativePreset(interrupted, "prot pve"), "inactive native slot accepted");

        std::string host, port, path;
        Require(SplitUrl("https://api.example.test", host, port, path) && host == "api.example.test" &&
            port == "443" && path.empty(), "root endpoint would create a double slash");
        Require(SplitUrl("https://proxy.example.test:8443/base/", host, port, path) &&
            port == "8443" && path == "/base", "proxy endpoint prefix lost");
        Require(!SplitUrl("http://example.test", host, port, path), "insecure transport accepted");
        Require(!SplitUrl("https://user:secret@example.test", host, port, path), "URL credentials accepted");
        Require(!SplitUrl("https://example.test:99999", host, port, path), "invalid port accepted");
        Require(!SplitUrl("https://example.test/#fragment", host, port, path), "URL fragment accepted");

        LlmBudget budget;
        Require(budget.Check(Purpose::Social, ModelTier::Fast) == BudgetResult::Disabled, "default budget enabled");
        budget.Configure(60, 100, 1);
        budget.SetEnabled(true);
        budget.OnQueued();
        Require(budget.Check(Purpose::Social, ModelTier::Fast) != BudgetResult::Allowed, "queue limit ignored");
        budget.OnDequeued();
        LlmResponse response;
        response.ok = true;
        response.promptTokens = 80;
        budget.Record(Purpose::Social, ModelTier::Fast, response, false);
        Require(budget.Check(Purpose::Plan, ModelTier::Think) == BudgetResult::Degraded, "think tier budget ignored");
        response.promptTokens = 20;
        budget.Record(Purpose::Social, ModelTier::Fast, response, false);
        Require(budget.Check(Purpose::Social, ModelTier::Fast) == BudgetResult::TokenBudgetExceeded, "token limit ignored");
        budget.SetEnabled(false);
        Require(budget.Check(Purpose::Social, ModelTier::Fast) == BudgetResult::Disabled, "pause budget ignored");

        LlmBudget rate;
        rate.Configure(60, 0, 0);
        rate.SetEnabled(true);
        auto const start = std::chrono::steady_clock::now();
        for (int i = 0; i < 60; ++i)
            Require(rate.Check(Purpose::Social, ModelTier::Fast, start) == BudgetResult::Allowed, "initial rate allowance lost");
        for (int i = 0; i < 5; ++i)
        {
            Require(rate.Check(Purpose::Social, ModelTier::Fast, start + std::chrono::milliseconds(i * 200)) == BudgetResult::CallRateExceeded, "rate ceiling ignored");
        }
        Require(rate.Check(Purpose::Social, ModelTier::Fast, start + std::chrono::milliseconds(1050)) == BudgetResult::Allowed, "fractional rate refill lost");

        Require(ValidateSchemasForStrictMode().empty(), "strict schemas invalid");
        Require(ValidateOutput("bot_utterance", R"({"line":"Hello","emote":null,"channel":"whisper"})").empty(), "valid response rejected");
        Require(!ValidateOutput("bot_utterance", R"({"line":"Hello","emote":null,"channel":"execute"})").empty(), "invalid channel accepted");
        Require(!ValidateOutput("bot_utterance", R"({"line":"Hello","channel":"say"})").empty(), "missing field accepted");
        Require(!ValidateOutput("bot_utterance", R"({"line":"Hello","emote":null,"channel":"say","command":"attack"})").empty(), "extra field accepted");
        Require(!ValidateOutput("bot_utterance", "malformed").empty(), "malformed JSON accepted");
        Require(!ValidateOutput("unknown", "{}").empty(), "unknown schema accepted");
        std::cout << "PASS: action authorization, budgets, queue/rate limits, schema rejection\n";
        return 0;
    }
    catch (std::exception const& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
