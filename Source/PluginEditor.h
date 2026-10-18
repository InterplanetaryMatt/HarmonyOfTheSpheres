#pragma once
#include "PluginProcessor.h"
#include <memory>
#include <vector>

class OrbitDisplay : public juce::Component, private juce::Timer
{
public:
 explicit OrbitDisplay(PlanetProcessor& p) : processor(p) { startTimerHz(30); }
 void paint(juce::Graphics&) override;
private:
 void timerCallback() override { repaint(); }
 PlanetProcessor& processor;
};

class PlanetEditor : public juce::AudioProcessorEditor
{
public:
 explicit PlanetEditor(PlanetProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
 using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
 PlanetProcessor& processor;
 OrbitDisplay orbits;
 juce::Label title, subtitle;
 juce::ComboBox mode, wave, spinNumber;
 juce::Slider earth, year, depth, ellipse, gain, duty;
 juce::ToggleButton audio, midi, bend, gate, spinCC;
 std::array<juce::ComboBox,8> channel;
 std::array<juce::ToggleButton,8> enabled;
 std::array<juce::Slider,8> level;
 std::array<juce::Label,8> planetName;
 std::vector<std::unique_ptr<ChoiceAttachment>> choices;
 std::vector<std::unique_ptr<SliderAttachment>> sliders;
 std::vector<std::unique_ptr<ButtonAttachment>> buttons;
 std::array<juce::Label,5> controlLabels;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlanetEditor)
};
