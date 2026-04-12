#include "base_delay.h"

BaseDelayProcessor::BaseDelayProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){}
BaseDelayEditor::BaseDelayEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){
    addAndMakeVisible(mTitle);
    mTitle.setText("Delay Effect", juce::dontSendNotification);

    addAndMakeVisible(mOpenCloseButton);
    //使按钮具有切换状态的功能
    mOpenCloseButton.setClickingTogglesState(true);
    mOpenCloseButton.onClick = [this]{
        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
    };

    addAndMakeVisible(mDelayTimeLabel);
    mDelayTimeLabel.setText("Delay Time (ms)", juce::dontSendNotification);
    
    addAndMakeVisible(mDelayTimeSlider);

    addAndMakeVisible(mWetLevelLabel);
    mWetLevelLabel.setText("Wet Level", juce::dontSendNotification);
    addAndMakeVisible(mWetLevelSlider);

    addAndMakeVisible(mDryLevelLabel);
    mDryLevelLabel.setText("Dry Level", juce::dontSendNotification);

    addAndMakeVisible(mDryLevelSlider);

    addAndMakeVisible(mFeedbackLabel);
    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);

    addAndMakeVisible(mFeedbackSlider);

    bindParameters();
}

void BaseDelayProcessor::createParameterLayout(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters)
{

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        openParamId,
        "Delay Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { delayTimeParamId, 1 },
        "Delay Time",
        juce::NormalisableRange<float>(50.0f, maxDelayTimeMs, 0.1f),
        350.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { wetLevelParamId, 1 },
        "Delay Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.35f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { dryLevelParamId, 1 },
        "Delay Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { feedbackParamId, 1 },
        "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.7f, 0.01f),
        0.5f));

}

void BaseDelayEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        openParamId,
        mOpenCloseButton);

    mDelayTimeAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        delayTimeParamId,
        mDelayTimeSlider);

    mWetLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        wetLevelParamId,
        mWetLevelSlider);

    mDryLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        dryLevelParamId,
        mDryLevelSlider);

    mFeedbackAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        feedbackParamId,
        mFeedbackSlider);

}

void BaseDelayEditor::resized()
{
    
    mTitle.setBounds(10,10,80,50);    
    mOpenCloseButton.setBounds(10, 70, 80, 30);    
    mDelayTimeLabel.setBounds(100, 10, 80, 30);    
    mDelayTimeSlider.setBounds(190,10, 200, 30);    
    mWetLevelLabel.setBounds(100, 50, 80, 30);    
    mWetLevelSlider.setBounds(190, 50, 200, 30);    
    mDryLevelLabel.setBounds(100, 90, 80, 30);    
    mDryLevelSlider.setBounds(190, 90, 200, 30);
    mFeedbackLabel.setBounds(100, 130, 80, 30);
    mFeedbackSlider.setBounds(190, 130, 200, 30);
}

void BaseDelayProcessor::syncParametersFromAPVTS()
{
    //if语句用来判断ID正确以及参数是否已经被注册
    if (auto* openParameter = mAPVTS.getRawParameterValue(openParamId))
        mParameters.isOpen = openParameter->load() >= 0.5f;

    if (auto* delayTimeParameter = mAPVTS.getRawParameterValue(delayTimeParamId))
        mParameters.delayTimeMs = delayTimeParameter->load();

    if (auto* wetLevelParameter = mAPVTS.getRawParameterValue(wetLevelParamId))
        mParameters.wetLevel = wetLevelParameter->load();

    if (auto* dryLevelParameter = mAPVTS.getRawParameterValue(dryLevelParamId))
        mParameters.dryLevel = dryLevelParameter->load();

    if (auto* feedbackParameter = mAPVTS.getRawParameterValue(feedbackParamId))
        mParameters.feedback = feedbackParameter->load();
}

//初始化
void BaseDelayProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels)
{
    mProcessor.prepareToPlay(sampleRate, maximumBlockSize, numChannels);
    syncParametersFromAPVTS();
    updateProcessorParameters();
}

void BaseDelayProcessor::DelayProcessor::prepareToPlay(
    double sampleRate,
    int maximumBlockSize,
    int numChannels)
{
    mCurrentSampleRate = sampleRate;

    // 计算最大延迟样本数，并设置延迟缓冲区的大小
    const auto maxDelaySamples =
        static_cast<int>(
            std::ceil((maxDelayTimeMs / 1000.0f) * static_cast<float>(sampleRate)));

    mDelayBufferLength = maxDelaySamples + maximumBlockSize + 1;
    mDelayBuffer.setSize(numChannels, mDelayBufferLength);
    mDelayBuffer.clear();

    mWritePosition = 0;

    mSmoothedDelayTimeMs.reset(sampleRate, 0.02);
    mSmoothedWetLevel.reset(sampleRate, 0.02);
    mSmoothedDryLevel.reset(sampleRate, 0.02);
    mSmoothedFeedback.reset(sampleRate, 0.02);

}

void BaseDelayProcessor::updateProcessorParameters()
{
    mProcessor.setParameters(mParameters);
}



void BaseDelayProcessor::DelayProcessor::setParameters(const DelayParameters& parameters)
{
    mSmoothedDelayTimeMs.setTargetValue(
        juce::jlimit(1.0f, maxDelayTimeMs, parameters.delayTimeMs));

    mSmoothedWetLevel.setTargetValue(
        juce::jlimit(0.0f, 1.0f, parameters.wetLevel));

    mSmoothedDryLevel.setTargetValue(
        juce::jlimit(0.0f, 1.0f, parameters.dryLevel));

    mSmoothedFeedback.setTargetValue(
        juce::jlimit(0.0f, 0.7f, parameters.feedback));

}

//得到实际的延迟样本数
float BaseDelayProcessor::DelayProcessor::getDelaySamples(float delayTimeMs) const
{
    const auto clampedDelayMs = juce::jlimit(1.0f, maxDelayTimeMs, delayTimeMs);

    //这里返回的是实际的延迟样本数
    return juce::jlimit(
        1.0f,
        static_cast<float>(mDelayBufferLength - 1),
        (clampedDelayMs / 1000.0f) * static_cast<float>(mCurrentSampleRate));
}

void BaseDelayProcessor::processDelay(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    syncParametersFromAPVTS();
    updateProcessorParameters();

    if (!mParameters.isOpen)
        return;

    mProcessor.processBlock(buffer, startSample, numSamples, numChannels);
}

//核心效果器处理部分
void BaseDelayProcessor::DelayProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    if (mDelayBufferLength <= 0)
        return;

    const auto delaySamples = getDelaySamples(mSmoothedDelayTimeMs.getNextValue());
    const auto wetLevel = mSmoothedWetLevel.getNextValue();
    const auto dryLevel = mSmoothedDryLevel.getNextValue();
    const auto feedback = mSmoothedFeedback.getNextValue();

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {   
        
        //此处获得几百毫秒前的历史音频样本
        float readPosition = static_cast<float>(mWritePosition) - delaySamples;   
        readPosition = getCircularBufferIndex(readPosition, mDelayBufferLength);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = 
                buffer.getWritePointer(channel, startSample);
            auto* delayData = mDelayBuffer.getWritePointer(channel);

            const float drySample = channelData[sampleIndex];
            const float delayedSample = getLinearInterpolator(
                delayData, 
                mDelayBufferLength, 
                readPosition);

            delayData[mWritePosition] = drySample + delayedSample * feedback;
            channelData[sampleIndex] = drySample * dryLevel + delayedSample * wetLevel;
        }

        ++mWritePosition;
        mWritePosition = getCircularBufferIndex(
            mWritePosition, 
            mDelayBufferLength);
    }
}



