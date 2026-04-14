cmake_minimum_required(VERSION 3.22)

project(AUDIO_PLUGIN_EXAMPLE VERSION 0.0.1)

# 默认关闭（即默认使用本地 JUCE）
option(USE_FETCHCONTENT "Set to ON to download JUCE automatically from GitHub" OFF)

if(USE_FETCHCONTENT)
    message(STATUS "Fetching JUCE from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.5  
    )
    FetchContent_MakeAvailable(JUCE)
else()
    message(STATUS "Looking for local JUCE installation...")
    find_package(JUCE CONFIG REQUIRED)
endif()


juce_add_plugin(Multi_effects_processor
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "YOK"                          # Specify the name of the plugin's author
    IS_SYNTH FALSE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE             # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE              # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE   
    COPY_PLUGIN_AFTER_BUILD False        
    PLUGIN_MANUFACTURER_CODE Yuse               
    PLUGIN_CODE K0yi                            
                                                
    FORMATS Standalone #VST3 AU #如果想要生成AU插件，取消注释AU，并确保在MacOS上构建
                               #如果想要生成VST3插件，取消注释VST3 
    PRODUCT_NAME "Multi_effects_processor")       

target_sources(Multi_effects_processor
    PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp)

add_subdirectory(Utils)
add_subdirectory(plugins)

target_include_directories(Multi_effects_processor
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})     

target_compile_definitions(Multi_effects_processor
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Multi_effects_processor
    PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        # 功能模块
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_basics    
        juce::juce_dsp             
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_generate_juce_header(Multi_effects_processor)
