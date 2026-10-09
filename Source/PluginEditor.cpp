#include "PluginProcessor.h"
#include "PluginEditor.h"

RingModSidechainAudioProcessorEditor::RingModSidechainAudioProcessorEditor (RingModSidechainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 500);
    historyMain.resize(600, 0.0f);
    historySidechain.resize(600, 0.0f);
    
    // Timer für 30 FPS Grafik-Update
    startTimerHz(30);
}

RingModSidechainAudioProcessorEditor::~RingModSidechainAudioProcessorEditor() {}

void RingModSidechainAudioProcessorEditor::timerCallback()
{
    // Werte in Historie schieben (für Bild 2: Linien von Rechts nach Links)
    std::rotate(historyMain.begin(), historyMain.begin() + 1, historyMain.end());
    historyMain.back() = audioProcessor.currentMainLevel.load();
    
    std::rotate(historySidechain.begin(), historySidechain.begin() + 1, historySidechain.end());
    historySidechain.back() = audioProcessor.currentSidechainLevel.load();
    
    repaint();
}

void RingModSidechainAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB(20, 20, 25)); // Dunkler Hintergrund

    auto bounds = getLocalBounds();
    auto envArea = bounds.removeFromTop(200).reduced(10); // Oberer Bereich (Bild 2)
    auto scopeArea = bounds.removeFromTop(200).reduced(10); // Unterer Bereich (Bild 1)

    // === Feature 2: Envelope Trace (Linien von Rechts nach Links) ===
    g.setColour(juce::Colours::black);
    g.fillRect(envArea);
    
    juce::Path pathMain, pathSC;
    for (int i = 0; i < envArea.getWidth(); ++i)
    {
        // Greife auf die letzten X Pixel in der Historie zu
        float m = historyMain[historyMain.size() - envArea.getWidth() + i];
        float sc = historySidechain[historySidechain.size() - envArea.getWidth() + i];
        
        float yM = envArea.getBottom() - (m * envArea.getHeight());
        float ySC = envArea.getBottom() - (sc * envArea.getHeight());
        
        if (i == 0) { pathMain.startNewSubPath(envArea.getX(), yM); pathSC.startNewSubPath(envArea.getX(), ySC); }
        else { pathMain.lineTo(envArea.getX() + i, yM); pathSC.lineTo(envArea.getX() + i, ySC); }
    }
    
    g.setColour(juce::Colours::orange.withAlpha(0.8f)); // Sound A (Orange)
    g.strokePath(pathMain, juce::PathStrokeType(2.0f));
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));   // Sound B (Cyan)
    g.strokePath(pathSC, juce::PathStrokeType(2.0f));

    // === Feature 1: Waveform / Phase Alignment (Übereinander) ===
    g.setColour(juce::Colour::fromRGB(15, 15, 20));
    g.fillRect(scopeArea);
    
    // Hinweis: In einem fertigen Produkt würde man hier über den visualBufferMain/Sidechain iterieren
    // und eine Sinus-Welle zeichnen, die an Nulldurchgängen verankert ist (Triggered Oscilloscope).
    // Zur Demonstration des Layouts und der Farben (Rot/Blau):
    g.setColour(juce::Colours::red.withAlpha(0.6f));
    g.drawHorizontalLine(scopeArea.getCentreY(), scopeArea.getX(), scopeArea.getRight()); // Platzhalter Phase Red
    g.setColour(juce::Colours::blue.withAlpha(0.6f));
    g.drawHorizontalLine(scopeArea.getCentreY() + 5, scopeArea.getX(), scopeArea.getRight()); // Platzhalter Phase Blue
    
    g.setColour(juce::Colours::white);
    g.drawText("RingMod Sidechain V3 - Crossover & Envelopes Active", getLocalBounds().removeFromBottom(50), juce::Justification::centred);
}

void RingModSidechainAudioProcessorEditor::resized() {}
