#include "PluginProcessor.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    mBaseDelayEditor(p.getAPVTS()),
    mBaseTremoloEditor(p.getAPVTS()),
    mSineSurroundEditor(p.getAPVTS()),
    mYOK3508Editor(p.getAPVTS()),
    mBaseOverdriveEditor(p.getAPVTS()),
    mBaseEQEditor(p.getAPVTS())
{
    outputGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    outputGainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (outputGainSlider);

    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getAPVTS(), 
        "outputGain", 
        outputGainSlider);

    addAndMakeVisible(testButton);
    testButton.setButtonText("midi test on");
    testButton.onClick = [this]() { 
        processorRef.isMidiTestOn = !processorRef.isMidiTestOn;
        testButton.setButtonText(processorRef.isMidiTestOn ? "midi test off" : "midi test on");
    };

    addAndMakeVisible(mBaseDelayEditor);
    addAndMakeVisible(mBaseTremoloEditor);
    addAndMakeVisible(mSineSurroundEditor);
    addAndMakeVisible(mYOK3508Editor);
    addAndMakeVisible(mBaseOverdriveEditor);
    addAndMakeVisible(mBaseEQEditor);
    setSize (1200, 700);
    
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    outputGainSlider.setBounds(10,10,100,50);
    testButton.setBounds(120, 10, 100, 30);
    mBaseDelayEditor.setBounds(10, 70, 380, 200);
    mBaseTremoloEditor.setBounds(400, 70, 380, 200);
    mSineSurroundEditor.setBounds(790, 70, 380, 200);
    mYOK3508Editor.setBounds(10, 280, 380, 300);
    mBaseOverdriveEditor.setBounds(400, 280, 380, 300);
    mBaseEQEditor.setBounds(790, 280, 380, 300);
}
