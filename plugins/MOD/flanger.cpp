#include "flanger.h"
#include "constants.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_core/juce_core.h"
#include "table.h"
#include "mathFunc.h"
#include <algorithm>

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

    addAndMakeVisible(baseDelayLabel);
    baseDelayLabel.setText("Pre-Delay", juce::dontSendNotification);
    addAndMakeVisible(baseDelaySlider);

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
    openCloseAttachment = std::make_unique<ButtonAttachment>(mAPVTS, FlangerOpenId, openCloseButton);
    depthAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerDepthId, depthSlider);
    wetAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerWetId, wetSlider);
    dryAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerDryId, drySlider);
    rateAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerRateId, rateSlider);
    feedbackAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerFeedbackId, feedbackSlider);
    baseDelayAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerPreDelayId, baseDelaySlider);
    phaseOffsetAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerPhaseOffsetId, phaseOffsetSlider);
    feedbackDampAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerFeedbackDampId, feedbackDampSlider);
    feedbackPolarityAttachment = std::make_unique<ButtonAttachment>(mAPVTS, FlangerFeedbackPolarityId, feedbackPolarityButton);
    lfoShapeSymmetryAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerLfoShapeSymmetryId, lfoShapeSymmetrySlider);
    lfoShapeStruationAttachment = std::make_unique<SliderAttachment>(mAPVTS, FlangerLfoShapeStruationId, lfoShapeStruationSlider);
}

void FlangerEditor::resized(){

    title.setBounds(10,10,100,30);
    openCloseButton.setBounds(10, 50, 100, 30);
    feedbackPolarityLabel.setBounds(10, 90, 100, 30);
    feedbackPolarityButton.setBounds(10, 130, 100, 30);

    depthLabel.setBounds(120,10,80,30);
    depthSlider.setBounds(200,10,150,30);

    wetLabel.setBounds(120,50,80,30);
    wetSlider.setBounds(200,50,150,30);

    dryLabel.setBounds(120,90,80,30);
    drySlider.setBounds(200,90,150,30);

    rateLabel.setBounds(120,130,80,30);
    rateSlider.setBounds(200,130,150,30);

    feedbackLabel.setBounds(120,170,80,30);
    feedbackSlider.setBounds(200,170,150,30);

    baseDelayLabel.setBounds(120,210,80,30);
    baseDelaySlider.setBounds(200,210,150,30);

    phaseOffsetLabel.setBounds(120,250,80,30);
    phaseOffsetSlider.setBounds(200,250,150,30);

    feedbackDampLabel.setBounds(120,290,80,30);
    feedbackDampSlider.setBounds(200,290,150,30);

    lfoShapeSymmetryLabel.setBounds(120,330,80,30);
    lfoShapeSymmetrySlider.setBounds(200,330,150,30);

    lfoShapeStruationLabel.setBounds(120,370,80,30);
    lfoShapeStruationSlider.setBounds(200,370,150,30);
}

void FlangerProcessor::createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &parameters){
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        FlangerOpenId, "Flanger Open", false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerDepthId, "Flanger Depth", juce::NormalisableRange<float>(0.1f, FlangerMaxLfoDepthMs, 0.01f), 4.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerWetId, "Flanger Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerDryId, "Flanger Dry", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    juce::NormalisableRange<float> rateHzRange(0.05f, 20.0f, 0.01f);
    rateHzRange.setSkewForCentre(2.0f);
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerRateId, "Flanger Rate", rateHzRange, 0.35f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerFeedbackId, "Flanger Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.5f));
    juce::NormalisableRange<float> preDelayMsRange(0.05f, FlangerMaxWetBaseDelayTimeMs, 0.001f);
    preDelayMsRange.setSkewForCentre(2.0f);
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerPreDelayId, "Flanger Pre-Delay", preDelayMsRange, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerPhaseOffsetId, "Flanger Phase Offset", juce::NormalisableRange<float>(0.0f, 360.0f, 1.0f), 180.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerFeedbackDampId, "Flanger Feedback Damp", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        FlangerFeedbackPolarityId, "Flanger Feedback Polarity", false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerLfoShapeSymmetryId, "Flanger LFO Shape Symmetry", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        FlangerLfoShapeStruationId, "Flanger LFO Shape Struation", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
}

void FlangerProcessor::syncParametersFromAPVTS(){
    if(auto* openParam = mAPVTS.getRawParameterValue(FlangerOpenId)){
        bool isOpen = openParam->load() >= 0.5f;
        smoothOpenClose.setTargetValue(isOpen ? 0.0f : 1.0f);
    }
    if(auto* depthParam = mAPVTS.getRawParameterValue(FlangerDepthId))
        depthMs = depthParam->load();
    if(auto* wetParam = mAPVTS.getRawParameterValue(FlangerWetId))
        wetLevel = wetParam->load();
    if(auto* dryParam = mAPVTS.getRawParameterValue(FlangerDryId))
        dryLevel = dryParam->load();
    if(auto* rateParam = mAPVTS.getRawParameterValue(FlangerRateId))
        rateHz = rateParam->load();
    if(auto* feedbackParam = mAPVTS.getRawParameterValue(FlangerFeedbackId))
        feedback = feedbackParam->load();
    if(auto* preDelayParam = mAPVTS.getRawParameterValue(FlangerPreDelayId))
        baseDelayMs = preDelayParam->load();
    if(auto* phaseOffsetParam = mAPVTS.getRawParameterValue(FlangerPhaseOffsetId))
        LFOPhaseOffset = phaseOffsetParam->load();
    if(auto* feedbackDampParam = mAPVTS.getRawParameterValue(FlangerFeedbackDampId))
        feedbackDamp = feedbackDampParam->load();
    if(auto* feedbackPolarityParam = mAPVTS.getRawParameterValue(FlangerFeedbackPolarityId))
        smoothFeedbackPolarity.setTargetValue(feedbackPolarityParam->load() >= 0.5f ? 1.0f : -1.0f);
    if(auto* lfoShapeSymmetryParam = mAPVTS.getRawParameterValue(FlangerLfoShapeSymmetryId))
        lfoShapeSymmetry = lfoShapeSymmetryParam->load();
    if(auto* lfoShapeStruationParam = mAPVTS.getRawParameterValue(FlangerLfoShapeStruationId))
        lfoShapeStruation = lfoShapeStruationParam->load();

}

void FlangerProcessor::updateProcessorParameters(){
    smoothDepth.setTargetValue(depthMs);
    smoothRate.setTargetValue(rateHz);
    smoothWet.setTargetValue(wetLevel);
    smoothDry.setTargetValue(dryLevel);
    smoothFeedback.setTargetValue(feedback);
    smoothBaseDelay.setTargetValue(baseDelayMs);
    smoothPhaseOffset.setTargetValue(LFOPhaseOffset);
    smoothFeedbackDamp.setTargetValue(feedbackDamp);
    smoothLfoShapeSymmetry.setTargetValue(lfoShapeSymmetry);
    smoothLfoShapeStruation.setTargetValue(lfoShapeStruation);
}

void FlangerProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels){

    currentSampleRate = sampleRate;

    smoothOpenClose.reset(currentSampleRate, 0.02);
    smoothDepth.reset(currentSampleRate, 0.02);
    smoothRate.reset(currentSampleRate, 0.02);
    smoothWet.reset(currentSampleRate, 0.02);
    smoothDry.reset(currentSampleRate, 0.02);
    smoothFeedback.reset(currentSampleRate, 0.02);
    smoothBaseDelay.reset(currentSampleRate, 0.02);
    smoothPhaseOffset.reset(currentSampleRate, 0.02);
    smoothFeedbackDamp.reset(currentSampleRate, 0.02);
    smoothFeedbackPolarity.reset(currentSampleRate, 0.02);
    smoothLfoShapeSymmetry.reset(currentSampleRate, 0.02);
    smoothLfoShapeStruation.reset(currentSampleRate, 0.02);

    syncParametersFromAPVTS();
    updateProcessorParameters();

    const float maxWetDelayMs = FlangerMaxWetBaseDelayTimeMs + FlangerMaxLfoDepthMs +FlangerDryPreDelayTimeMs;
    //预延迟的最大时间加上最大深度加上干信号预延迟时间
    const float maxNumWetDelaySamples = ms2Samples(maxWetDelayMs, currentSampleRate);
    //将最大延迟毫秒数转化为样本数

    flangerStates.resize(numChannels);

    for(int channel = 0; channel < numChannels; channel++){
        flangerStates[channel].lfoPhase = 0;
        flangerStates[channel].writeIndex = 0;
        flangerStates[channel].lowpass.prepareToPlay(feedbackDamp);
        flangerStates[channel].wetDelayBuffer.resize(maxNumWetDelaySamples, 0.0f);
        flangerStates[channel].preDelayState.prepareToPlay(FlangerDryPreDelayTimeMs, currentSampleRate);
        flangerStates[channel].preDelayState.setValue(currentSampleRate, FlangerDryPreDelayTimeMs);
    }

}

void FlangerProcessor::processFlanger(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels){

    syncParametersFromAPVTS();
    updateProcessorParameters();
    
    if(smoothOpenClose.getTargetValue() == 1.0f && smoothOpenClose.getCurrentValue() > 0.999f)
        return;

    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++){

        float currentBypassGain = smoothOpenClose.getNextValue();
        float currentDepthMs = smoothDepth.getNextValue();
        float currentWetLevel = smoothWet.getNextValue();
        float currentDryLevel = smoothDry.getNextValue();
        float currentFeedback = smoothFeedback.getNextValue();
        float currentPhaseOffset = smoothPhaseOffset.getNextValue();
        float currentFeedbackDamp = smoothFeedbackDamp.getNextValue();
        float currentFeedbackPolarity = smoothFeedbackPolarity.getNextValue();
        float currentLfoShapeSymmetry = smoothLfoShapeSymmetry.getNextValue();
        float currentLfoShapeStruation = smoothLfoShapeStruation.getNextValue();
        float currentRateHz = smoothRate.getNextValue();
        float currentPreDelayMs = smoothBaseDelay.getNextValue();

        for(int channel = 0; channel < numChannels; channel++){

            if(smoothFeedbackDamp.isSmoothing())
                flangerStates[channel].lowpass.setValue(currentFeedbackDamp);

            auto* channelData = buffer.getWritePointer(channel, startSample);
            float phaseStep = currentRateHz / currentSampleRate * 360.0f;//计算LFO步长
            float lfoValueMs = processLFO(
                getCircularBufferIndex(flangerStates[channel].lfoPhase + channel * currentPhaseOffset, 360), 
                currentLfoShapeSymmetry, 
                currentLfoShapeStruation) * currentDepthMs;
            float delayMs = currentPreDelayMs + lfoValueMs + FlangerDryPreDelayTimeMs;//计算湿信号延迟时间
            delayMs = std::max(delayMs, 0.5f);//防止采集到未来样本
            float numDelaySamples = ms2Samples(delayMs, currentSampleRate); //毫秒数转化为样本数
            float readIndex = getCircularBufferIndex(
                flangerStates[channel].writeIndex - numDelaySamples, flangerStates[channel].wetDelayBuffer.size());
            float readSample = getLagrangeInterpolator(
                flangerStates[channel].wetDelayBuffer.data(), 
                flangerStates[channel].wetDelayBuffer.size(), 
                readIndex);
            float rawSample = channelData[sampleIndex];//原始数据
            float drySample = rawSample;
            
            float feedbackInput = readSample * currentFeedback * currentFeedbackPolarity;//未经过阻尼的反馈数据
            feedbackInput = flangerStates[channel].lowpass.processSample(feedbackInput);
            //获取经过反馈阻尼处理后的湿数据
            flangerStates[channel].wetDelayBuffer[flangerStates[channel].writeIndex] = rawSample + feedbackInput;//将湿数据写入延迟缓冲区
            rawSample = flangerStates[channel].preDelayState.processSample(rawSample);//干数据进行预延迟处理
            
            channelData[sampleIndex] = rawSample * currentDryLevel + readSample *currentWetLevel;//将反馈阻尼后的数据写回去,并进行干湿混合
            flangerStates[channel].writeIndex++;
            flangerStates[channel].writeIndex = getCircularBufferIndex(
                flangerStates[channel].writeIndex, 
                flangerStates[channel].wetDelayBuffer.size());
            flangerStates[channel].lfoPhase += phaseStep;
            flangerStates[channel].lfoPhase = getCircularBufferIndex(flangerStates[channel].lfoPhase, 360);

            channelData[sampleIndex] = (1.0f - currentBypassGain) * channelData[sampleIndex] + currentBypassGain * drySample;//开关状态平滑处理
            
        }
    }
    
    
}

float FlangerProcessor::processLFO(float phase, float symmetry, float saturation)
{
    //将参数映射到 0.0 - 1.0 方便计算
    float phasePoint = juce::jmap(phase, 0.0f, 360.0f, 0.0f, 1.0f);
    float symPoint = juce::jmap(symmetry, -1.0f, 1.0f, 0.01f, 0.99f);
    
    // 计算非对称三角波 (范围 0 to 1)
    float triangle;
    if (phasePoint < symPoint)
        triangle = phasePoint / symPoint;
    else
        triangle = (1.0f - phasePoint) / (1.0f - symPoint);
        
    // 转为 -1.0 到 1.0
    float rawWave = 2.0f * triangle - 1.0f;

    // 应用饱和度
    // 当 saturation=0 时，drive=1.0；当 saturation=100 时，drive 很大
    float drive = juce::jmap(saturation, 0.0f, 100.0f, 1.0f, 5.0f);
    
    // 使用 tanh 塑形
    float shapedWave = tanhApproximate(drive * rawWave) / tanhApproximate(drive);

    return shapedWave;
}