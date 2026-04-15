#include "multiChannelsChorus.h"
#include "Utils/table.h"
#include "constants.h"
#include "Utils/mathFunc.h"
#include "juce_audio_basics/juce_audio_basics.h"
YOK3508Processor::YOK3508Processor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{

}
YOK3508Editor::YOK3508Editor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
	addAndMakeVisible(mTitle);
	mTitle.setText("3channel Chorus", juce::dontSendNotification);

    mOpenCloseButton.setClickingTogglesState(true);
	addAndMakeVisible(mOpenCloseButton);
	mOpenCloseButton.onClick = [this]{

        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
	};

	addAndMakeVisible(mDepthLabel);
	mDepthLabel.setText("Depth", juce::dontSendNotification);
	addAndMakeVisible(mDepthSlider);

	addAndMakeVisible(mRateLabel);
	mRateLabel.setText("Rate", juce::dontSendNotification);
	addAndMakeVisible(mRateSlider);

    addAndMakeVisible(mMixLabel);
    mMixLabel.setText("Mix", juce::dontSendNotification);
    addAndMakeVisible(mMixSlider);

    addAndMakeVisible(mFeedbackLabel);
    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    addAndMakeVisible(mFeedbackSlider);

    addAndMakeVisible(mBaseDelayLabel);
    mBaseDelayLabel.setText("Base Delay", juce::dontSendNotification);
    addAndMakeVisible(mBaseDelaySlider);

    bindParameters();
}

void YOK3508Editor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS, ThreeChannelsChorusOpenId, mOpenCloseButton);

    mDepthAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, ThreeChannelsChorusDepthId, mDepthSlider);

    mRateAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, ThreeChannelsChorusRateId, mRateSlider);

    mMixAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, ThreeChannelsChorusMixId, mMixSlider);

    mFeedbackAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, ThreeChannelsChorusFeedbackId, mFeedbackSlider);

    mBaseDelayAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, ThreeChannelsChorusBaseDelayId, mBaseDelaySlider);
}

void YOK3508Processor::createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &parameters){
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        ThreeChannelsChorusOpenId,
        "3Channel Chorus Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusDepthId, 1 },
        "3Channel Chorus Depth",
        juce::NormalisableRange<float>(0, maxSineDepthMs, 0.1f),
        4.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusRateId, 1 },
        "3Channel Chorus Rate",
        juce::NormalisableRange<float>(0.01f, 5.0f, 0.01f),
        0.35f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusMixId, 1 },
        "3Channel Chorus Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusFeedbackId, 1 },
        "3Channel Chorus Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusBaseDelayId, 1 },
        "3Channel Chorus Base Delay",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f),
        4.0f));
}

void YOK3508Processor::syncParametersFromAPVTS()
{
    if (auto* openParameter = mAPVTS.getRawParameterValue(ThreeChannelsChorusOpenId))
        mIsOpen = openParameter->load() >= 0.5f;

    if (auto* depthParameter = mAPVTS.getRawParameterValue(ThreeChannelsChorusDepthId))
        mDepthMs = depthParameter->load();

    if (auto* rateParameter = mAPVTS.getRawParameterValue(ThreeChannelsChorusRateId))
        mRateHz = rateParameter->load();

    if (auto* mixParameter = mAPVTS.getRawParameterValue(ThreeChannelsChorusMixId))
        mMix = mixParameter->load();

    if (auto* feedbackParameter = mAPVTS.getRawParameterValue(ThreeChannelsChorusFeedbackId))
        mFeedback = feedbackParameter->load();

    if(auto* baseDelayParameter = mAPVTS.getRawParameterValue(ThreeChannelsChorusBaseDelayId))
        mBaseDelayMs = baseDelayParameter->load();
}

void YOK3508Editor::resized()
{
	mTitle.setBounds(10, 10, 100, 30);
	mOpenCloseButton.setBounds(10, 50, 100, 30);

	mDepthSlider.setBounds(220,10,150,30);
	mDepthLabel.setBounds(130,10,80,30);

	mRateSlider.setBounds(220,50,150,30);
	mRateLabel.setBounds(130,50,110,30);

    mMixSlider.setBounds(220,90,150,30);
    mMixLabel.setBounds(130,90,80,30);

    mFeedbackSlider.setBounds(220,130,150,30);
    mFeedbackLabel.setBounds(130,130,80,30);

    mBaseDelaySlider.setBounds(220,170,150,30);
    mBaseDelayLabel.setBounds(130,170,80,30);
}

void YOK3508Processor::prepareToPlay(
	double sampleRate,
	int maximumBlockSize,
	int numChannels)
{
	mCurrentSampleRate = sampleRate;

	const auto maxDelaySamples = static_cast<int>(std::ceil(
		((mBaseDelayMs + maxSineDepthMs) / 1000.0f) *
		static_cast<float>(sampleRate)));

	mDelayBufferLength = maxDelaySamples + maximumBlockSize + 1;

	int delayChannels{numChannels};
	if(delayChannels < 2){
		delayChannels = 2;
	}

	mDelayBuffer.setSize(delayChannels, mDelayBufferLength);
	mDelayBuffer.clear();

	mWritePosition = 0;
	mSineTableIndex = 0.0f;
	SineLookUpTable(mSineLookUpTable,bufferSize);

	mSmoothedDepthMs.reset(sampleRate, 0.02);
	mSmoothedMix.reset(sampleRate, 0.02);
    mSmoothedRateHz.reset(sampleRate, 0.02);
    mSmoothedFeedback.reset(sampleRate, 0.02);
    mSmoothedBaseDelayMs.reset(sampleRate, 0.02);

    syncParametersFromAPVTS();
	mUpdateProcessorParameters();

    //不要在processBlock里创建buffer，这样会导致每次处理音频时都重新分配内存，效率极低
    wetBuffer.setSize(3, maximumBlockSize);
}

void YOK3508Processor::mUpdateProcessorParameters()
{
	mSmoothedDepthMs.setTargetValue(mDepthMs);
	mSmoothedRateHz.setTargetValue(mRateHz);
	mSmoothedMix.setTargetValue(mMix);
	mSmoothedFeedback.setTargetValue(mFeedback);
	mSmoothedBaseDelayMs.setTargetValue(mBaseDelayMs);
}

void YOK3508Processor::processThreeChannelsChorus(
	juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples,
	int numChannels)
{
    syncParametersFromAPVTS();
    mUpdateProcessorParameters();

	if(!mIsOpen){
		return;
	}

	if(mDelayBufferLength <= 0 || numChannels < 2 || mSineLookUpTable.empty()){
		return;
	}

    //处理第一组声道（0和1），相位偏移为0
    processCertainChorus(buffer, 
        wetBuffer.getWritePointer(0), 
        0.0f, 0.0f, startSample, numSamples);

    //相位偏移为120度（2*pi/3），右通道偏移左通道5°（two_pi/72）
    processCertainChorus(buffer, 
        wetBuffer.getWritePointer(1), 
        transformRadIntoMs(two_pi / 3.0f, mCurrentSampleRate), 
        two_pi / 72.0f, 
        startSample, numSamples);

    //相位偏移为240度（4*pi/3），右通道偏移左通道10°（two_pi/36）
    processCertainChorus(buffer, 
        wetBuffer.getWritePointer(2), 
        transformRadIntoMs(2.0f * two_pi / 3.0f, mCurrentSampleRate), 
        two_pi / 36.0f, 
        startSample, numSamples);

    
}



void YOK3508Processor::processCertainChorus(
	juce::AudioBuffer<float>& buffer,
	float* wetBufferData,
	float phaseOffsetMs,//单支路左右通道偏移毫秒数(用来确定这条支路的具体声音方位)（时间轴）
    float rightRadToLeftRad, //右声道相对于左声道的正弦波相位偏移弧度数（用来实现合唱的流动感）（信号轴）
	int startSample,
	int numSamples)
{
    auto* leftChannelData = buffer.getReadPointer(0, startSample);
    auto* rightChannelData = buffer.getReadPointer(1, startSample);
    auto* leftDelayData = mDelayBuffer.getWritePointer(0);
    auto* rightDelayData = mDelayBuffer.getWritePointer(1);

	for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++){
		const float currentDepthMs = mSmoothedDepthMs.getNextValue();
		const float currentRateHz = mSmoothedRateHz.getNextValue();
        const float currentMix = mSmoothedMix.getNextValue();
        const float currentFeedback = mSmoothedFeedback.getNextValue();
        const float currentBaseDelayMs = mSmoothedBaseDelayMs.getNextValue();

        float sineValueLeft = getLinearInterpolator(mSineLookUpTable.data(), 
            static_cast<int>(mSineLookUpTable.size()), 
            mSineTableIndex);

        float sineValueRight = getLinearInterpolator(mSineLookUpTable.data(), 
            static_cast<int>(mSineLookUpTable.size()), 
            mSineTableIndex + transformRadIntoIndexStep(rightRadToLeftRad, mSineLookUpTable.size()));

		float leftDelayMs = currentBaseDelayMs + currentDepthMs * sineValueLeft + phaseOffsetMs / 2;
		float rightDelayMs = currentBaseDelayMs + currentDepthMs * sineValueRight - phaseOffsetMs / 2;

		float leftReadPosition =
			static_cast<float>(mWritePosition) - transformMsIntoSamples(leftDelayMs, mCurrentSampleRate);
		float rightReadPosition =
			static_cast<float>(mWritePosition) - transformMsIntoSamples(rightDelayMs, mCurrentSampleRate);

        //调节索引防止越界
        leftReadPosition = getCircularBufferIndex(
            leftReadPosition, static_cast<float>(mDelayBufferLength));
        rightReadPosition = getCircularBufferIndex(
            rightReadPosition, static_cast<float>(mDelayBufferLength));

        //获取当前输入的左右声道样本(原始样本)
		const float leftInputSample = leftChannelData[sampleIndex];
		const float rightInputSample = rightChannelData[sampleIndex];

        //原始样本存储，待未来使用
		leftDelayData[mWritePosition] = leftInputSample;
		rightDelayData[mWritePosition] = rightInputSample;

		leftChannelData[sampleIndex] = getLinearInterpolator(
			leftDelayData,
            mDelayBufferLength,
			leftReadPosition) * currentWetLevel +
            leftInputSample * currentDryLevel;
		rightChannelData[sampleIndex] = getLinearInterpolator(
			rightDelayData,
            mDelayBufferLength,
			rightReadPosition) * currentWetLevel +
            rightInputSample * currentDryLevel;

		mSineTableIndex +=
			(currentPhaseFrequencyHz / static_cast<float>(mCurrentSampleRate)) *
			static_cast<float>(mSineLookUpTable.size());

        mSineTableIndex = getCircularBufferIndex(
            mSineTableIndex, static_cast<float>(mSineLookUpTable.size()));

		mWritePosition++;
        mWritePosition = getCircularBufferIndex(mWritePosition, mDelayBufferLength);

	}
}



