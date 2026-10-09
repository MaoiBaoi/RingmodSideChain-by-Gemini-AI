#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class RingModSidechainAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    RingModSidechainAudioProcessorEditor (RingModSidechainAudioProcessor&);
    ~RingModSidechainAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    RingModSidechainAudioProcessor& audioProcessor;
    
    // Sliders
    juce::Slider crossSlider, attackSlider, holdSlider, releaseSlider, wetSlider;
    
    // Historie für das "Linien von Rechts nach Links" Feature (Bild 2)
    std::vector<float> historyMain;
    std::vector<float> historySidechain;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RingModSidechainAudioProcessorEditor)
};
