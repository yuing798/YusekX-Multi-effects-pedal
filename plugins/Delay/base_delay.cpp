#include "base_delay.h"
#include "Utils/mathFunc.h"
#include "Utils/constants.h"

//效果器构造函数
BaseDelayProcessor::BaseDelayProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){}

//效果器界面构造函数
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

//将ID和APVTS绑定
void BaseDelayProcessor::createParameterLayout(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters)
{

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        BaseDelayOpenId,
        "Delay Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseDelayTimeId, 1 },
        "Delay Time",
        juce::NormalisableRange<float>(50.0f, baseDelaymaxDelayTimeMs, 0.1f),
        350.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseDelayWetId, 1 },
        "Delay Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.35f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseDelayDryId, 1 },
        "Delay Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseDelayFeedbackId, 1 },
        "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.7f, 0.01f),
        0.5f));

}

//将UI滑块和APVTS参数绑定
void BaseDelayEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        BaseDelayOpenId,
        mOpenCloseButton);

    mDelayTimeAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseDelayTimeId,
        mDelayTimeSlider);

    mWetLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseDelayWetId,
        mWetLevelSlider);

    mDryLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseDelayDryId,
        mDryLevelSlider);

    mFeedbackAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseDelayFeedbackId,
        mFeedbackSlider);

}

//组件位置
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

//将DSP参数和APVTS参数同步
void BaseDelayProcessor::syncParametersFromAPVTS()
{
    //if语句用来判断ID正确以及参数是否已经被注册
    if (auto* openParameter = mAPVTS.getRawParameterValue(BaseDelayOpenId))
        isOpen = openParameter->load() >= 0.5f;

    if (auto* delayTimeParameter = mAPVTS.getRawParameterValue(BaseDelayTimeId))
        delayTimeMs = delayTimeParameter->load();

    if (auto* wetLevelParameter = mAPVTS.getRawParameterValue(BaseDelayWetId))
        wetLevel = wetLevelParameter->load();

    if (auto* dryLevelParameter = mAPVTS.getRawParameterValue(BaseDelayDryId))
        dryLevel = dryLevelParameter->load();

    if (auto* feedbackParameter = mAPVTS.getRawParameterValue(BaseDelayFeedbackId))
        feedback = feedbackParameter->load();
}

//初始化
void BaseDelayProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels)
{
    mCurrentSampleRate = sampleRate;

    // 计算最大延迟样本数，并设置延迟缓冲区的大小
    const auto maxDelaySamples =
        static_cast<int>(
            std::ceil((baseDelaymaxDelayTimeMs / 1000.0f) * static_cast<float>(sampleRate)));

    mDelayBufferLength = maxDelaySamples + maximumBlockSize + 1;
    mDelayBuffer.setSize(numChannels, mDelayBufferLength);
    mDelayBuffer.clear();

    mWritePosition = 0;

    mSmoothedDelayTimeMs.reset(sampleRate, 0.02);
    mSmoothedWetLevel.reset(sampleRate, 0.02);
    mSmoothedDryLevel.reset(sampleRate, 0.02);
    mSmoothedFeedback.reset(sampleRate, 0.02);

    //初始化同步
    syncParametersFromAPVTS();
    updateProcessorParameters();
}


void BaseDelayProcessor::updateProcessorParameters()
{
    mSmoothedDelayTimeMs.setTargetValue(delayTimeMs);
    mSmoothedWetLevel.setTargetValue(wetLevel);
    mSmoothedDryLevel.setTargetValue(dryLevel);
    mSmoothedFeedback.setTargetValue(feedback);
}

//得到实际的延迟样本数
float BaseDelayProcessor::getDelaySamples(float delayTimeMs) const
{
    //这里返回的是实际的延迟样本数
    return juce::jlimit(
        1.0f,
        static_cast<float>(mDelayBufferLength - 1),
        (delayTimeMs / 1000.0f) * static_cast<float>(mCurrentSampleRate));
}

void BaseDelayProcessor::processDelay(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    //逐buffer同步参数
    //sync函数如果放在isOpen后面会发生死锁问题，
    // 因为当插件关闭时，APVTS参数会被重置为默认值，如果此时不进行同步，
    // 参数值将无法更新，导致isOpen状态无法正确反映，从而无法退出循环。
    syncParametersFromAPVTS();
    updateProcessorParameters();//平滑度更新不用放在for循环中

    if (!isOpen)
        return;

    processBlock(buffer, startSample, numSamples, numChannels);
}

//核心效果器处理部分
void BaseDelayProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    if (mDelayBufferLength <= 0)
        return;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {   
        const auto delaySamples = getDelaySamples(mSmoothedDelayTimeMs.getNextValue());
        const auto wetValue = mSmoothedWetLevel.getNextValue();
        const auto dryValue = mSmoothedDryLevel.getNextValue();
        const auto feedbackValue = mSmoothedFeedback.getNextValue();
            
        //此处获得几百毫秒前的历史音频样本
        float readPosition = static_cast<float>(mWritePosition) - delaySamples;   
        readPosition = getCircularBufferIndex(readPosition, mDelayBufferLength);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = 
                buffer.getWritePointer(channel, startSample);
            auto* delayData = mDelayBuffer.getWritePointer(channel);

            const float drySample = channelData[sampleIndex];
            //线性插值获取延迟缓冲区中对应位置的样本值
            const float delayedSample = getLinearInterpolator(
                delayData, 
                mDelayBufferLength, 
                readPosition);

            delayData[mWritePosition] = drySample + delayedSample * feedbackValue;
            channelData[sampleIndex] = drySample * dryValue + delayedSample * wetValue;
        }

        ++mWritePosition;
        mWritePosition = getCircularBufferIndex(
            mWritePosition, 
            mDelayBufferLength);
    }
}



