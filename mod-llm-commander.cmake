if("${MODULE_MOD-LLM-COMMANDER}" STREQUAL "disabled" OR MODULES STREQUAL "none")
  return()
endif()
set(LLMC_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(LLMC_PLAYERBOTS_ROOT "${CMAKE_SOURCE_DIR}/modules/mod-playerbots" CACHE PATH "External Playerbots module")
# The embedded script registers .llmbot even when LlmBots.Enable is false.
if(EXISTS "${LLMC_PLAYERBOTS_ROOT}/src/LlmBots/Script/LlmBotsScript.cpp")
  message(FATAL_ERROR "Embedded LlmBots also registers .llmbot. Use Playerbots without embedded LlmBots; disabling its config is insufficient. Do not install both implementations.")
endif()
if("${MODULE_MOD-PLAYERBOTS}" STREQUAL "disabled")
  message(FATAL_ERROR "LLM Commander requires enabled mod-playerbots.")
endif()
if(TARGET mod_mod-llm-commander OR TARGET mod_mod-playerbots)
  message(FATAL_ERROR "LLM Commander currently supports static linking only. Set MODULES=static and both MODULE_mod-* entries to static/default. Playerbots cross-DLL exports are not verified.")
endif()
foreach(header PlayerbotAIConfig.h Script/Playerbots.h Bot/PlayerbotAI.h Bot/PlayerbotMgr.h Bot/RandomPlayerbotMgr.h Bot/Engine/ExternalEventHelper.h Bot/Engine/WorldPacket/Event.h Script/WorldThr/PlayerbotOperations.h Script/WorldThr/PlayerbotWorldThreadProcessor.h)
  if(NOT EXISTS "${LLMC_PLAYERBOTS_ROOT}/src/${header}")
    message(FATAL_ERROR "Unsupported Playerbots layout: missing ${header}. Add a compatibility adapter; do not patch Playerbots.")
  endif()
endforeach()
file(READ "${LLMC_PLAYERBOTS_ROOT}/src/Bot/PlayerbotMgr.h" LLMC_MANAGER_HEADER)
file(READ "${LLMC_PLAYERBOTS_ROOT}/src/Script/Playerbots.h" LLMC_SCRIPT_HEADER)
if(LLMC_MANAGER_HEADER MATCHES "GetPlayerbotAI")
  set(LLMC_LOOKUP_MANAGER 1)
elseif(LLMC_SCRIPT_HEADER MATCHES "GET_PLAYERBOT_AI")
  set(LLMC_LOOKUP_MANAGER 0)
else()
  message(FATAL_ERROR "Unsupported Playerbots AI lookup: neither GetPlayerbotAI nor GET_PLAYERBOT_AI is available.")
endif()
configure_file("${LLMC_ROOT}/cmake/CommanderCompatibility.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/llm-commander/CommanderCompatibility.h" @ONLY)
CollectIncludeDirectories("${LLMC_PLAYERBOTS_ROOT}/src" LLMC_PB_INCLUDES)
target_include_directories(modules PRIVATE "${LLMC_ROOT}/deps"
  "${CMAKE_CURRENT_BINARY_DIR}/llm-commander" ${LLMC_PB_INCLUDES})
target_link_libraries(modules PRIVATE openssl)
message(STATUS "LLM Commander: static; manager lookup=${LLMC_LOOKUP_MANAGER}. Interface assertions run during compilation.")

# Ship the standalone schema without connecting to or modifying a database.
install(DIRECTORY "${LLMC_ROOT}/data/sql/" DESTINATION "sql/mod-llm-commander")

# Add local setup tools to the existing Windows build/install layout.
# No server binary, active configuration or database is copied/changed by this target.
if(WIN32 AND TARGET worldserver)
  set(LLMC_SETUP_FILES
    "${LLMC_ROOT}/tools/windows/LLM-SETUP.bat"
    "${LLMC_ROOT}/tools/windows/LLM-SETUP.ps1"
    "${LLMC_ROOT}/tools/windows/LLM-SETUP-KO.md")
  # The inspected core sets this to <build>/bin for MSVC and MinGW.
  # Multi-config generators add the configuration directory, as worldserver does.
  set(LLMC_SETUP_OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  if(NOT LLMC_SETUP_OUTPUT)
    message(FATAL_ERROR "Windows setup tools require the core runtime output directory.")
  endif()
  if(CMAKE_CONFIGURATION_TYPES)
    string(APPEND LLMC_SETUP_OUTPUT "/$<CONFIG>")
  endif()
  add_custom_target(llm_commander_setup_tools ALL
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${LLMC_SETUP_OUTPUT}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${LLMC_SETUP_FILES} "${LLMC_SETUP_OUTPUT}"
    DEPENDS ${LLMC_SETUP_FILES}
    COMMENT "Copying LLM Commander local setup tools"
    VERBATIM)
  add_dependencies(worldserver llm_commander_setup_tools)
  install(FILES ${LLMC_SETUP_FILES} DESTINATION ".")
endif()
