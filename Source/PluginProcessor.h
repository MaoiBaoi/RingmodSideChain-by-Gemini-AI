#pragma once
#include <JuceHeader.h>

class RingModSidechainAudioProcessor : public juce::AudioProcessor
{
public:
    RingModSidechainAudioProcessor();
    ~RingModSidechainAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "RingMod Sidechain V3"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}
    void getStateInformation (juce::MemoryBlock& destData) override {}
    void setStateInformation (const void* data, int sizeInBytes) override {}

    juce::AudioProcessorValueTreeState apvts;

    // Für die GUI Visualisierung
    std::atomic<float> currentMainLevel { 0.0f };
    std::atomic<float> currentSidechainLevel { 0.0f };
    juce::AudioBuffer<float> visualBufferMain;
    juce::AudioBuffer<float> visualBufferSidechain;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    // DSP Module
    juce::dsp::LinkwitzRileyFilter<float> crossoverLow;
    juce::dsp::LinkwitzRileyFilter<float> crossoverHigh;
    
    float envelopeLevel = 0.0f;
    int holdCounter = 0;
    float sampleRate = 44100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RingModSidechainAudioProcessor)
};
