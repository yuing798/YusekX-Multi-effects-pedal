target_sources(Multi_effects_processor PRIVATE
    table.cpp
    table.h
    constants.h
    mathFunc.cpp
    mathFunc.h
    UtilsEffect.h
    UtilsEffect.cpp
    dspFilters.h
    dspFilters.cpp
    dataStructs.h
    dataStructs.cpp)

target_include_directories(Multi_effects_processor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})