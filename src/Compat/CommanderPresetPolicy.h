/* GPL-2.0-or-later. Match the native preset lookup before mutating a bot. */
#pragma once

#include <span>
#include <string>
#include <string_view>

namespace LlmCommander::Compat
{
inline bool ContainsNativePreset(std::span<std::string const> presets, std::string_view requested)
{
    if (requested.empty())
        return false;
    for (auto const& preset : presets)
    {
        // Native SpecPick stops at the first unconfigured slot.
        if (preset.empty())
            break;
        if (std::string_view(preset) == requested)
            return true;
    }
    return false;
}
}
