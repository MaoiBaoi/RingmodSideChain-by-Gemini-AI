#include "PluginProcessor.h"
#include "PluginEditor.h"

RingModSidechainAudioProcessor::RingModSidechainAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameters())
{
    visualBufferMain.setSize(1, 1024);
    visualBufferSidechain.setSize(1, 1024);
}

juce::AudioProcessorValueTreeState::ParameterLayout RingModSidechainAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("THRESH", "Threshold", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CROSS", "Duck Under Hz", 20.0f, 1000.0f, 150.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack (ms)", 0.0f, 50.0f, 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("HOLD", "Hold (ms)", 0.0f, 50.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release (ms)", 0.0f, 200.0f, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("WET", "Wet %", 0.0f, 1.0f, 1.0f));
    return { params.begin(), params.end() };
}

void RingModSidechainAudioProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    juce::dsp::ProcessSpec spec { sr, static_cast<juce::uint32>(samplesPerBlock), 2 };
    crossoverLow.prepare(spec);
    crossoverLow.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    crossoverHigh.prepare(spec);
    crossoverHigh.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
}

void RingModSidechainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    auto scBus = getBusBuffer(buffer, true, 1); // Sidechain Eingang
    auto mainBus = getBusBuffer(buffer, true, 0); // Main Eingang
    
    float thresh = apvts.getRawParameterValue("THRESH")->load();
    float crossFreq = apvts.getRawParameterValue("CROSS")->load();
    float attack = apvts.getRawParameterValue("ATTACK")->load();
    float hold = apvts.getRawParameterValue("HOLD")->load();
    float release = apvts.getRawParameterValue("RELEASE")->load();
    float wet = apvts.getRawParameterValue("WET")->load();

    crossoverLow.setCutoffFrequency(crossFreq);
    crossoverHigh.setCutoffFrequency(crossFreq);

    float attackCoeff = std::exp(-1.0f / (attack * 0.001f * sampleRate + 1.0f));
    float releaseCoeff = std::exp(-1.0f / (release * 0.001f * sampleRate + 1.0f));
    int holdSamples = static_cast<int>(hold * 0.001f * sampleRate);

    // Crossover berechnen
    juce::AudioBuffer<float> lowBuffer;
    lowBuffer.makeCopyOf(mainBus);
    juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
    crossoverLow.process(juce::dsp::ProcessContextReplacing<float>(lowBlock));

    juce::AudioBuffer<float> highBuffer;
    highBuffer.makeCopyOf(mainBus);
    juce::dsp::AudioBlock<float> highBlock(highBuffer);
    crossoverHigh.process(juce::dsp::ProcessContextReplacing<float>(highBlock));

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        // 1. Sidechain Signal auslesen (Mono-Mix)
        float scSample = 0.0f;
        if (scBus.getNumChannels() > 0)
            scSample = std::abs(scBus.getSample(0, i));
            
        // Attack / Hold / Release Logik
        if (scSample > envelopeLevel) {
            envelopeLevel = attackCoeff * envelopeLevel + (1.0f - attackCoeff) * scSample;
            holdCounter = holdSamples;
        } else {
            if (holdCounter > 0) holdCounter--;
            else envelopeLevel = releaseCoeff * envelopeLevel + (1.0f - releaseCoeff) * scSample;
        }

        // 2. Ringmod Mathematik: Ducking Faktor berechnen
        float duckFactor = 1.0f - (envelopeLevel * (1.0f / juce::jmax(thresh, 0.001f)));
        duckFactor = juce::jlimit(0.0f, 1.0f, duckFactor);

        // 3. Auf Main Signal (nur Low Band) anwenden
        for (int ch = 0; ch < mainBus.getNumChannels(); ++ch)
        {
            float orig = mainBus.getSample(ch, i);
            float low = lowBuffer.getSample(ch, i);
            float high = highBuffer.getSample(ch, i);
            
            float duckedLow = low * duckFactor;
            float processed = duckedLow + high; // Lows geduckt, Highs bleiben unberührt!
            
            mainBus.setSample(ch, i, orig * (1.0f - wet) + processed * wet);
        }

        // Für die GUI speichern (Scope + Envelope)
        if (i % 44 == 0) { // Spart CPU für GUI
            currentMainLevel = std::abs(mainBus.getSample(0, i));
            currentSidechainLevel = scSample;
        }
    }
}

juce::AudioProcessorEditor* RingModSidechainAudioProcessor::createEditor() { return new RingModSidechainAudioProcessorEditor (*this); }
