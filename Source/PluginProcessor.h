#pragma once
#include <JuceHeader.h>
#include <array>
class PlanetProcessor final : public juce::AudioProcessor
{
public:
 PlanetProcessor();
 void prepareToPlay(double, int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "Harmony of the Spheres"; }
 bool acceptsMidi() const override { return true; }
 bool producesMidi() const override { return true; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 0.0; }
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return {}; }
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*, int) override;
 bool isBusesLayoutSupported(const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
 juce::AudioProcessorValueTreeState parameters;
 double getOrbitTime() const noexcept { return orbitTime.load(std::memory_order_relaxed); }
 int getReferenceMidiNote() const noexcept { return displayedNote.load(std::memory_order_relaxed); }
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
 std::array<double,8> phase{};
 std::array<bool,128> heldNotes{};
 std::array<unsigned long long,128> noteOrder{};
 unsigned long long nextNoteOrder=0;
 int referenceNote=-1;
 std::array<int,8> lastExpression{}, lastPan{};
 double ccElapsed=0;
 std::atomic<int> displayedNote{-1};
 std::array<int,8> activeNote{};
 std::array<int,8> activeChannel{};
 std::atomic<double> orbitTime{0.0};
 std::array<double,8> smoothLevel{};
 double elapsedSeconds=0, sampleRateHz=44100;
 bool wasSending=false;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlanetProcessor)
};
