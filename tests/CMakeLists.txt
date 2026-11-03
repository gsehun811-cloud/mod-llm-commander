cmake_minimum_required(VERSION 3.16)
project(CommanderOfflineTests LANGUAGES CXX)
enable_testing()
add_executable(commander_offline_tests commander_offline_tests.cpp
  ../src/Llm/CommanderLlmBudget.cpp ../src/Llm/CommanderLlmSchema.cpp ../src/Llm/CommanderUrl.cpp)
target_compile_features(commander_offline_tests PRIVATE cxx_std_20)
target_include_directories(commander_offline_tests PRIVATE ../src/Llm ../src/Compat ../deps)
add_test(NAME commander_offline COMMAND commander_offline_tests)
