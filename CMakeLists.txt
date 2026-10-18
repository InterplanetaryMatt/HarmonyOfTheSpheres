cmake_minimum_required(VERSION 3.22)
project(HarmonyOfTheSpheres VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(HarmonyOfTheSpheres
    COMPANY_NAME InterplanetaryMatt
    BUNDLE_ID com.interplanetarymatt.harmonyofthespheres
    PLUGIN_MANUFACTURER_CODE InMt
    PLUGIN_CODE HoSp
    FORMATS VST3 Standalone
    PRODUCT_NAME "Harmony of the Spheres"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(HarmonyOfTheSpheres)
target_sources(HarmonyOfTheSpheres PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_definitions(HarmonyOfTheSpheres PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(HarmonyOfTheSpheres PRIVATE juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
