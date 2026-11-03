"""Read-only source comparison. Does not configure, compile, connect or start servers."""
import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re


def read(path):
    return path.read_text(encoding="utf-8-sig")


def normalize(text):
    text = re.sub(r"Commander(?=Llm|OpenAi|Foundry|Mock|Https|Prompt|Chronicler)", "", text)
    return text.replace("LlmCommander", "LlmBots").replace("llmcommander", "llmbots").replace("llmc_", "llm_")


def commands(text):
    return re.findall(r'\{\s*"(\w+)",\s*(Handle\w+),\s*(rbac::\w+),\s*(Console::\w+)', text)


def extract(text, start, end):
    return text[text.index(start):text.index(end, text.index(start))]


def without_comments(text):
    # Preserve literals while removing comments; raw tool-schema strings contain no comments.
    pattern = r'R"([^()\\ \t\r\n]{0,16})\(.*?\)\1"|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*'
    return re.sub(pattern, lambda m: "" if m.group().startswith(("/*", "//")) else m.group(), text,
                  flags=re.S).strip()


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("original", type=Path, help="Read-only original src/LlmBots directory")
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
source = root / "src"
files = []
for original in sorted(args.original.rglob("*")):
    if not original.is_file():
        continue
    relative = original.relative_to(args.original)
    independent = source / relative.parent / ("Commander" + relative.name)
    assert independent.is_file(), f"Missing extracted file: {relative}"
    before, after = read(original), normalize(read(independent))
    diff = list(difflib.unified_diff(before.splitlines(), after.splitlines(), n=0))
    files.append({"original": relative.as_posix(), "independent": independent.relative_to(root).as_posix(),
                  "original_sha256": hashlib.sha256(original.read_bytes()).hexdigest(),
                  "normalized_diff_lines": len(diff),
                  "implementation_identical_after_naming": without_comments(before) == without_comments(after)})

original_script = read(args.original / "Script/LlmBotsScript.cpp")
raw_script = read(source / "Script/CommanderLlmBotsScript.cpp")
script = normalize(raw_script)
table_start, table_end = "    ChatCommandTable GetCommands()", "    static bool HandleLlmBotList("
assert extract(original_script, table_start, table_end) == extract(raw_script, table_start, table_end), \
    "Raw command tables differ: added/removed roots, aliases, handlers or permissions"
command_scripts = []
for cpp in source.rglob("*.cpp"):
    for name in re.findall(r'class\s+(\w+)\s*:\s*public\s+CommandScript\b', without_comments(read(cpp))):
        command_scripts.append((cpp.relative_to(root).as_posix(), name))
assert len(command_scripts) == 1, "Additional CommandScript registration found"
assert commands(original_script) == commands(script), "Command name/handler/RBAC/console changed"
assert len(commands(script)) == 10
assert '{ "llmbot", llmbotCommands }' in script
handlers_start = "    static bool HandleLlmBotList("
assert extract(original_script, handlers_start, "} // namespace") == extract(script, handlers_start, "} // namespace"), \
    "Command arguments or handler bodies changed"
assert script.count("void OnPlayerLogin(") == script.count("void OnPlayerLogout(") == 1
handler_signatures = re.findall(r'    static bool (HandleLlmBot\w+\([^\n]+\))', raw_script)
assert handler_signatures == re.findall(r'    static bool (HandleLlmBot\w+\([^\n]+\))', original_script)

original_config = read(args.original / "LlmBotsConfig.cpp")
config = read(source / "CommanderLlmBotsConfig.cpp")
original_keys = re.findall(r'GetOption<[^>]+>\("LlmBots\.([^"\n]+)"', original_config)
new_keys = re.findall(r'ReadOption<[^>]+>\("([^"\n]+)"', config)
assert original_keys == new_keys, "Configuration key lost or reordered"
original_defaults = re.findall(
    r'cfg\.(\w+)\s*=\s*sConfigMgr->GetOption<([^>]+)>\("LlmBots\.([^"]+)",\s*(.*?)\);', original_config)
new_defaults = re.findall(r'cfg\.(\w+)\s*=\s*ReadOption<([^>]+)>\("([^"]+)",\s*(.*?)\);', config)
assert original_defaults == new_defaults, "Original code-level default changed"
template = read(root / "conf/mod_llm_commander.conf.dist")
template_keys = re.findall(r'^(Llm\w+\.[\w.]+)\s*=', template, re.M)
expected_keys = {prefix + key for prefix in ["LlmBots.", "LlmCommander."] for key in original_keys}
assert len(template_keys) == len(expected_keys) and set(template_keys) == expected_keys, \
    "Unexpected setting, alias or duplicate in template"
for key in original_keys:
    assert re.search(r"^LlmBots\." + re.escape(key) + r"\s*=", template, re.M)
    assert f"LlmCommander.{key} = @legacy@" in template

original_tools = read(args.original / "Bots/LlmToolbox.cpp")
tools = normalize(read(source / "Bots/CommanderLlmToolbox.cpp"))
tool_start, tool_end = "std::vector<ToolSpec> const& SocialToolSpecs()", "std::string ExecuteTool("
assert extract(original_tools, tool_start, tool_end) == extract(tools, tool_start, tool_end), \
    "Advertised tool names, arguments, descriptions or allocation changed"
tool_names = re.findall(r'out.push_back\(\{ "([^"]+)"', extract(tools, tool_start, tool_end))
assert len(tool_names) == 16

module = read(source / "Script/CommanderLlmBotsModule.cpp")
for marker in ["CommanderAccountOnline()", "EnsureBotsSpawned()", "Compat::QueueLogin(entry.guid)",
               "Compat::Logout(entry.guid)", "kWatchdogIntervalMs = 10000"]:
    assert marker in module
assert "only bots controlled by the issuer may receive commands" not in tools
assert "CanExecuteAction(" in tools
original_brain = read(args.original / "Bots/LlmBotBrain.cpp")
brain = normalize(read(source / "Bots/CommanderLlmBotBrain.cpp"))
agenda_start, agenda_end = "void LlmBotBrain::ApplyAgenda(", "void LlmBotBrain::FallbackLine("
original_agenda = extract(original_brain, agenda_start, agenda_end)
agenda = extract(brain, agenda_start, agenda_end)
original_toggle_gate = 'if (parsed.contains("strategy_toggles") && parsed["strategy_toggles"].is_array())'
commander_toggle_gate = ('if (EqualsIgnoreCase(m_name, m_commanderName) &&\n'
                         '            parsed.contains("strategy_toggles") && parsed["strategy_toggles"].is_array())')
assert agenda.count(commander_toggle_gate) == 1, "Ordinary bot strategy changes are not blocked"
expected_agenda = original_agenda.replace(original_toggle_gate, commander_toggle_gate)
assert without_comments(expected_agenda).split() == without_comments(
    agenda.replace("Compat::GetAI(player)", "GET_PLAYERBOT_AI(player)")).split(), \
    "Unexpected goal/agenda change beyond the Commander-only strategy gate"
original_module = read(args.original / "Script/LlmBotsModule.cpp")
backend_pattern = r'\nvoid LlmBotsModule::(Cmd\w+)\([^)]*\)\n\{.*?(?=\nvoid LlmBotsModule::|\n\} // namespace)'
original_backends = {m.group(1): m.group() for m in re.finditer(backend_pattern, original_module, re.S)}
normalized_module = normalize(module).replace("CharacterDatabase", "PlayerbotsDatabase").replace(".DirectExecute(", ".Execute(")
current_backends = {m.group(1): m.group() for m in re.finditer(backend_pattern, normalized_module, re.S)}
assert original_backends.keys() == current_backends.keys()
for name, body in original_backends.items():
    if name != "CmdPause":
        assert body == current_backends[name], f"Command backend changed: {name}"

print(json.dumps({"scope": "source inspection only; no compilation or runtime validation",
                  "command_count": len(commands(script)), "commands": commands(script),
                  "command_roots": ["llmbot"], "additional_game_commands_or_aliases": [],
                  "command_scripts": command_scripts, "handler_signatures": handler_signatures,
                  "backend_comparison": "CmdAdd/Remove/Persona/Stats/Say/Goal equal after DB adapter naming; CmdPause retains safety guards",
                  "agenda_policy": "ordinary bot strategy changes blocked; original Commander manual goal retained",
                  "config_key_count": len(original_keys), "config_suffixes": original_keys,
                  "tool_count": len(tool_names), "tools": tool_names, "files": files}, indent=2))
