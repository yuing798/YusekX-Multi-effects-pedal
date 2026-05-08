#include "flanger.h"
#include "constants.h"
#include "juce_audio_processors/juce_audio_processors.h"

FlangerProcessor::FlangerProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
}
FlangerEditor::FlangerEditor(juce::AudioProcessorValueTreeState& apvts)
    :mAPVTS(apvts)
{
    addAndMakeVisible(title);
    title.setText("Flanger", juce::dontSendNotification);

    openCloseButton.setClickingTogglesState(true);
    addAndMakeVisible(openCloseButton);
    openCloseButton.onClick = [this]{
        const auto isOpen = openCloseButton.getToggleState();
        openCloseButton.setButtonText(isOpen ? "Close" : "Open");
    };

    addAndMakeVisible(depthLabel);
    depthLabel.setText("Depth", juce::dontSendNotification);
    addAndMakeVisible(depthSlider);

    addAndMakeVisible(wetLabel);
    wetLabel.setText("Wet", juce::dontSendNotification);
    addAndMakeVisible(wetSlider);

    addAndMakeVisible(dryLabel);
    dryLabel.setText("Dry", juce::dontSendNotification);
    addAndMakeVisible(drySlider);

    addAndMakeVisible(rateLabel);
    rateLabel.setText("Rate", juce::dontSendNotification);
    addAndMakeVisible(rateSlider);

    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    addAndMakeVisible(feedbackSlider);

    addAndMakeVisible(preDelayLabel);
    preDelayLabel.setText("Pre-Delay", juce::dontSendNotification);
    addAndMakeVisible(preDelaySlider);

    addAndMakeVisible(phaseOffsetLabel);
    phaseOffsetLabel.setText("Phase Offset", juce::dontSendNotification);
    addAndMakeVisible(phaseOffsetSlider);

    addAndMakeVisible(feedbackDampLabel);
    feedbackDampLabel.setText("Feedback Damp", juce::dontSendNotification);
    addAndMakeVisible(feedbackDampSlider);

    addAndMakeVisible(feedbackPolarityButton);
    feedbackPolarityButton.setClickingTogglesState(true);
    feedbackPolarityButton.onClick = [this]{
        const auto isPositive = feedbackPolarityButton.getToggleState();
        feedbackPolarityButton.setButtonText(isPositive ? "+" : "-");
    };

    addAndMakeVisible(lfoShapeSymmetryLabel);
    lfoShapeSymmetryLabel.setText("LFO Symmetry", juce::dontSendNotification);
    addAndMakeVisible(lfoShapeSymmetrySlider);

    addAndMakeVisible(lfoShapeStruationLabel);
    lfoShapeStruationLabel.setText("LFO Saturation", juce::dontSendNotification);
    addAndMakeVisible(lfoShapeStruationSlider);

    bindParameters();
}

void FlangerEditor::bindParameters()
{
    openCloseAttachment = std::make_unique<ButtonAttachment>(mAPVTS, "flangerOpen", openCloseButton);
    depthAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerDepth", depthSlider);
    wetAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerWet", wetSlider);
    dryAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerDry", drySlider);
    rateAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerRate", rateSlider);
    feedbackAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerFeedback", feedbackSlider);
    preDelayAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerPreDelay", preDelaySlider);
    phaseOffsetAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerPhaseOffset", phaseOffsetSlider);
    feedbackDampAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerFeedbackDamp", feedbackDampSlider);
    feedbackPolarityAttachment = std::make_unique<ButtonAttachment>(mAPVTS, "flangerFeedbackPolarity", feedbackPolarityButton);
    lfoShapeSymmetryAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerLfoShapeSymmetry", lfoShapeSymmetrySlider);
    lfoShapeStruationAttachment = std::make_unique<SliderAttachment>(mAPVTS, "flangerLfoShapeStruation", lfoShapeStruationSlider);
}

void FlangerEditor::resized(){

    title.setBounds(10,10,100,30);
    openCloseButton.setBounds(10, 50, 100, 30);
    feedbackPolarityLabel.setBounds(10, 90, 100, 30);
    feedbackPolarityButton.setBounds(10, 130, 100, 30);

    depthLabel.setBounds(120,10,80,30);
    depthSlider.setBounds(200,10,100,30);

    wetLabel.setBounds(120,50,80,30);
    wetSlider.setBounds(200,50,100,30);

    dryLabel.setBounds(120,90,80,30);
    drySlider.setBounds(200,90,100,30);

    rateLabel.setBounds(120,130,80,30);
    rateSlider.setBounds(200,130,100,30);

    feedbackLabel.setBounds(120,170,80,30);
    feedbackSlider.setBounds(200,170,100,30);

    preDelayLabel.setBounds(120,210,80,30);
    preDelaySlider.setBounds(200,210,100,30);

    phaseOffsetLabel.setBounds(120,250,80,30);
    phaseOffsetSlider.setBounds(200,250,100,30);

    feedbackDampLabel.setBounds(120,290,80,30);
    feedbackDampSlider.setBounds(200,290,100,30);

    lfoShapeSymmetryLabel.setBounds(120,330,80,30);
    lfoShapeSymmetrySlider.setBounds(200,330,100,30);

    lfoShapeStruationLabel.setBounds(120,370,80,30);
    lfoShapeStruationSlider.setBounds(200,370,100,30);
}

void FlangerProcessor::createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &parameters){
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("flangerOpen", "Flanger Open", false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerDepth", "Flanger Depth", juce::NormalisableRange<float>(0.1f, 30.0f, 0.1f), 4.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerWet", "Flanger Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerDry", "Flanger Dry", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerRate", "Flanger Rate", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerFeedback", "Flanger Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerPreDelay", "Flanger Pre-Delay", juce::NormalisableRange<float>(0.05f, 15.0f, 0.1f), 4.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerPhaseOffset", "Flanger Phase Offset", juce::NormalisableRange<float>(0.0f, 360.0f, 1.0f), 180.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerFeedbackDamp", "Flanger Feedback Damp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("flangerFeedbackPolarity", "Flanger Feedback Polarity", true));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerLfoShapeSymmetry", "Flanger LFO Shape Symmetry", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("flangerLfoShapeStruation", "Flanger LFO Shape Struation", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
}

void FlangerProcessor::syncParametersFromAPVTS(){
    if(auto* openParam = mAPVTS.getRawParameterValue("flangerOpen")){
        bool isOpen = openParam->load() >= 0.5f;
        smoothOpenClose.setTargetValue(isOpen ? 0.0f : 1.0f);
    }
    if(auto* depthParam = mAPVTS.getRawParameterValue("flangerDepth"))
        depthMs = depthParam->load();
    if(auto* wetParam = mAPVTS.getRawParameterValue("flangerWet"))
        wetLevel = wetParam->load();
    if(auto* dryParam = mAPVTS.getRawParameterValue("flangerDry"))
        dryLevel = dryParam->load();
    if(auto* rateParam = mAPVTS.getRawParameterValue("flangerRate"))
        rateHz = rateParam->load();
    if(auto* feedbackParam = mAPVTS.getRawParameterValue("flangerFeedback"))
        feedback = feedbackParam->load();
    if(auto* preDelayParam = mAPVTS.getRawParameterValue("flangerPreDelay"))
        preDelayMs = preDelayParam->load();
    if(auto* phaseOffsetParam = mAPVTS.getRawParameterValue("flangerPhaseOffset"))
        LFOPhaseOffset = phaseOffsetParam->load();
    if(auto* feedbackDampParam = mAPVTS.getRawParameterValue("flangerFeedbackDamp"))
        feedbackDamp = feedbackDampParam->load();
    if(auto* feedbackPolarityParam = mAPVTS.getRawParameterValue("flangerFeedbackPolarity"))
        smoothFeedbackPolarity.setTargetValue(feedbackPolarityParam->load() >= 0.5f ? 1.0f : -1.0f);
    if(auto* lfoShapeSymmetryParam = mAPVTS.getRawParameterValue("flangerLfoShapeSymmetry"))
        lfoShapeSymmetry = lfoShapeSymmetryParam->load();
    if(auto* lfoShapeStruationParam = mAPVTS.getRawParameterValue("flangerLfoShapeStruation"))
        lfoShapeStruation = lfoShapeStruationParam->load();

}

void FlangerProcessor::updateProcessorParameters(){
    smoothDepth.setTargetValue(depthMs);
    smoothRate.setTargetValue(rateHz);
    smoothWet.setTargetValue(wetLevel);
    smoothDry.setTargetValue(dryLevel);
    smoothFeedback.setTargetValue(feedback);
    smoothPreDelay.setTargetValue(preDelayMs);
    smoothPhaseOffset.setTargetValue(LFOPhaseOffset);
    smoothFeedbackDamp.setTargetValue(feedbackDamp);
    smoothLfoShapeSymmetry.setTargetValue(lfoShapeSymmetry);
    smoothLfoShapeStruation.setTargetValue(lfoShapeStruation);
}

void FlangerProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels){
    currentSampleRate = sampleRate;

    smoothDepth.reset(currentSampleRate, 0.02);
    smoothRate.reset(currentSampleRate, 0.02);
    smoothWet.reset(currentSampleRate, 0.02);
    smoothDry.reset(currentSampleRate, 0.02);
    smoothFeedback.reset(currentSampleRate, 0.02);
    smoothPreDelay.reset(currentSampleRate, 0.02);
    smoothPhaseOffset.reset(currentSampleRate, 0.02);
    smoothFeedbackDamp.reset(currentSampleRate, 0.02);
    smoothLfoShapeSymmetry.reset(currentSampleRate, 0.02);
    smoothLfoShapeStruation.reset(currentSampleRate, 0.02);

    syncParametersFromAPVTS();
    updateProcessorParameters();
}

void FlangerProcessor::processFlanger(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels){
    
    if(smoothOpenClose.getTargetValue() == 1.0f && smoothOpenClose.getCurrentValue() > 0.999f)
        return;
    processBlock(buffer, startSample, numSamples, numChannels);
}