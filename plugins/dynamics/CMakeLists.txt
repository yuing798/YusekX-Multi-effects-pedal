target_sources(Multi_effects_processor
    PRIVATE
        base_compressor.cpp
        base_compressor.h
        ducker.cpp
        ducker.h)

target_include_directories(Multi_effects_processor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})