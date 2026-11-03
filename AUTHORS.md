# Attribution

LLM code was extracted from the local `mod-playerbots/src/LlmBots` tree, preserving its notices.
No Playerbots engine sources were copied. The local tree also contains additional modifications whose
individual authorship cannot be recovered without repository history.

- Risto aMaazing: original named-roster LLM feature, upstream commit
  [3e41da940b9e8d44d3c41db91f9f6f41a394110a](https://github.com/gsehun811-cloud/mod-llm-playerbots-commander/commit/3e41da940b9e8d44d3c41db91f9f6f41a394110a).
- gsehun811-cloud: Windows LLM Commander customization and upstream credits, reference commit
  [45ac63a5b7a4182df22873f20b7834ca3afb3b9e](https://github.com/gsehun811-cloud/mod-llm-playerbots-commander/commit/45ac63a5b7a4182df22873f20b7834ca3afb3b9e).
- AzerothCore and mod-playerbots contributors: external core and Playerbots APIs.
- Niels Lohmann and nlohmann/json contributors: vendored `deps/json.hpp`, MIT license embedded in the header.

Independent packaging, compatibility boundary, permission/threading/budget/transport corrections,
offline tests and documentation were developed for this source extraction. No Git repository was
initialized and no commits, remote pushes or pull requests were created during this work.

The complete upstream contributors list is preserved in [UPSTREAM_AUTHORS.md](docs/UPSTREAM_AUTHORS.md).
Original distribution notices from reference 45ac63a were restored during the 2026-11-03 audit.
The 0.1.2-audit source restoration is not covered by the historical 0.1.1 build result.
