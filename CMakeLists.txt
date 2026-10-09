cmake_minimum_required(VERSION 3.20)
project(RingModSidechain VERSION 3.0.0)

# Lade das JUCE Framework herunter
include(FetchContent)
FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.0)
FetchContent_MakeAvailable(juce)

# Plugin definieren
juce_add_plugin(RingModSidechain
    COMPANY_NAME "DeinName"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE JucE
    PLUGIN_CODE Rmsc
    FORMATS VST3
    PRODUCT_NAME "RingMod Sidechain V3")

target_sources(RingModSidechain PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_features(RingModSidechain PRIVATE cxx_std_20)
target_link_libraries(RingModSidechain PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra)
