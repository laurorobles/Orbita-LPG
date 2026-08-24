cmake_minimum_required(VERSION 3.15)
project(OrbitaLPG VERSION 0.1.0)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.1.zip
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(OrbitaLPG
    COMPANY_NAME "Extasis Records"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE ExtR
    PLUGIN_CODE OrbL
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Orbita-LPG")

target_sources(OrbitaLPG PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(OrbitaLPG PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(OrbitaLPG PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)
juce_generate_juce_header(OrbitaLPG)
