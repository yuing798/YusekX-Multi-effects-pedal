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
    mBaseEQEditor(p.getAPVTS()),
    mBaseCompressorEditor(p.getAPVTS()),
    mSchroederReverbEditor(p.getAPVTS())
{
    effectSelector.addItem("base delay", 1);
    effectSelector.addItem("base tremolo", 2);
    effectSelector.addItem("sine surround", 3);
    effectSelector.addItem("YOK3508", 4);
    effectSelector.addItem("base overdrive", 5);
    effectSelector.addItem("base EQ", 6);
    effectSelector.addItem("base compressor", 7);
    effectSelector.addItem("Schroeder Reverb", 8);
    effectSelector.setSelectedId(1);
    addAndMakeVisible(effectSelector);
    effectSelector.onChange = [this]() { updateEffectEditor(); };
    addAndMakeVisible(mBaseDelayEditor);
    addAndMakeVisible(mBaseTremoloEditor);
    addAndMakeVisible(mSineSurroundEditor);
    addAndMakeVisible(mYOK3508Editor);
    addAndMakeVisible(mBaseOverdriveEditor);
    addAndMakeVisible(mBaseEQEditor);
    addAndMakeVisible(mBaseCompressorEditor);
    addAndMakeVisible(mSchroederReverbEditor);

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

    setSize (1200, 800);
    
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
    effectSelector.setBounds(230, 10, 150, 30);
    mBaseDelayEditor.setBounds(10, 70, 380, 300);
    mBaseTremoloEditor.setBounds(10, 70, 380, 300);
    mSineSurroundEditor.setBounds(10, 70, 380, 300);
    mYOK3508Editor.setBounds(10, 70, 380, 300);
    mBaseOverdriveEditor.setBounds(10, 70, 380, 300);
    mBaseEQEditor.setBounds(10, 70, 380, 300);
    mBaseCompressorEditor.setBounds(10, 70, 380, 300);
    mSchroederReverbEditor.setBounds(10, 70, 380, 300);
}

void AudioPluginAudioProcessorEditor::updateEffectEditor(){

    // 1. 隐藏所有编辑器
    mBaseDelayEditor.setVisible(false);
    mBaseTremoloEditor.setVisible(false);
    mSineSurroundEditor.setVisible(false);
    mYOK3508Editor.setVisible(false);
    mBaseOverdriveEditor.setVisible(false);
    mBaseEQEditor.setVisible(false);
    mBaseCompressorEditor.setVisible(false);
    mSchroederReverbEditor.setVisible(false);

    // 2. 只显示选中的编辑器
    switch (effectSelector.getSelectedId())
    {
        case 1: mBaseDelayEditor.setVisible(true); break;
        case 2: mBaseTremoloEditor.setVisible(true); break;
        case 3: mSineSurroundEditor.setVisible(true); break;
        case 4: mYOK3508Editor.setVisible(true); break;
        case 5: mBaseOverdriveEditor.setVisible(true); break;
        case 6: mBaseEQEditor.setVisible(true); break;
        case 7: mBaseCompressorEditor.setVisible(true); break;
        case 8: mSchroederReverbEditor.setVisible(true); break;
        default: break;
    }
}
