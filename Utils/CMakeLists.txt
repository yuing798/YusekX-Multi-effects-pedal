target_sources(Multi_effects_processor PRIVATE
    table.cpp
    table.h
    constants.h
    mathFunc.cpp
    mathFunc.h)

target_include_directories(Multi_effects_processor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})