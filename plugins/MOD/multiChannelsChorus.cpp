#include "multiChannelsChorus.h"
#include "Utils/table.h"
#include "constants.h"
#include "Utils/mathFunc.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <memory>

YOK3508Processor::YOK3508Processor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
    SineLookUpTable(mWetTable, bufferSize);
    CosLookUpTable(mDryTable, bufferSize);
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

    addAndMakeVisible(mPhaseOffsetLabel);
    mPhaseOffsetLabel.setText("Phase Offset", juce::dontSendNotification);
    addAndMakeVisible(mPhaseOffsetSlider);

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

    mPhaseOffsetAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, ThreeChannelsChorusPhaseOffsetId, mPhaseOffsetSlider);
}

void YOK3508Processor::createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &parameters){
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        ThreeChannelsChorusOpenId,
        "3Channel Chorus Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusDepthId, 1 },
        "3Channel Chorus Depth",
        juce::NormalisableRange<float>(0, 8.0f, 0.1f),
        4.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusRateId, 1 },
        "3Channel Chorus Rate",
        juce::NormalisableRange<float>(0.01f, 3.0f, 0.01f),
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
        juce::NormalisableRange<float>(5.0f, 20.0f, 0.1f),
        4.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ThreeChannelsChorusPhaseOffsetId, 1 },
        "3Channel Chorus Phase Offset",
        juce::NormalisableRange<float>(-30.0f, 30.0f, 0.1f),
        0.0f));
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

    if (auto* phaseOffsetParameter = mAPVTS.getRawParameterValue(ThreeChannelsChorusPhaseOffsetId))
        mPhaseOffsetMs = phaseOffsetParameter->load();
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

    mPhaseOffsetSlider.setBounds(220,210,150,30);
    mPhaseOffsetLabel.setBounds(130,210,80,30);
}

void YOK3508Processor::prepareToPlay(
	double sampleRate,
	int maximumBlockSize,
	int numChannels)
{
	mCurrentSampleRate = sampleRate;

	const auto maxDelaySamples = static_cast<int>(std::ceil(
		((mBaseDelayMs + maxSineDepthMs + 50.0f) / 1000.0f) * //这里的50ms是为了给phaseOffsetMs相位偏移预留的时间
		static_cast<float>(sampleRate)));

	mDelayBufferLength = maxDelaySamples + maximumBlockSize + 1;

	int delayChannels{numChannels};
	if(delayChannels < 2){
		delayChannels = 2;
	}

    
    chorusBranch1.mDelayBuffer.setSize(delayChannels, mDelayBufferLength);
    chorusBranch1.mDelayBuffer.clear();
    chorusBranch1.mWritePosition = 0;
    chorusBranch1.mSineTableIndex = 0.0f;

    chorusBranch2.mDelayBuffer.setSize(delayChannels, mDelayBufferLength);
    chorusBranch2.mDelayBuffer.clear();
    chorusBranch2.mWritePosition = 0;
    chorusBranch2.mSineTableIndex = 0.0f;

    chorusBranch3.mDelayBuffer.setSize(delayChannels, mDelayBufferLength);
    chorusBranch3.mDelayBuffer.clear();
    chorusBranch3.mWritePosition = 0;
    chorusBranch3.mSineTableIndex = 0.0f;

	SineLookUpTable(mSineLookUpTable,bufferSize);

	mSmoothedDepthMs.reset(sampleRate, 0.02);
	mSmoothedMix.reset(sampleRate, 0.02);
    mSmoothedRateHz.reset(sampleRate, 0.02);
    mSmoothedFeedback.reset(sampleRate, 0.02);
    mSmoothedBaseDelayMs.reset(sampleRate, 0.02);
    mSmoothedPhaseOffsetMs.reset(sampleRate, 0.02);

    syncParametersFromAPVTS();
	mUpdateProcessorParameters();

    //不要在processBlock里创建buffer，这样会导致每次处理音频时都重新分配内存，效率极低
    chorusBranch1.wetBuffer.setSize(6, maximumBlockSize);
    chorusBranch1.wetBuffer.clear();

    chorusBranch2.wetBuffer.setSize(2, maximumBlockSize);
    chorusBranch2.wetBuffer.clear();

    chorusBranch3.wetBuffer.setSize(2, maximumBlockSize);
    chorusBranch3.wetBuffer.clear();

    finalWetBuffer.setSize(2, maximumBlockSize);
    finalWetBuffer.clear();
}

void YOK3508Processor::mUpdateProcessorParameters()
{
	mSmoothedDepthMs.setTargetValue(mDepthMs);
	mSmoothedRateHz.setTargetValue(mRateHz);
	mSmoothedMix.setTargetValue(mMix);
	mSmoothedFeedback.setTargetValue(mFeedback);
	mSmoothedBaseDelayMs.setTargetValue(mBaseDelayMs);
	mSmoothedPhaseOffsetMs.setTargetValue(mPhaseOffsetMs);
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

    finalWetBuffer.clear();//每次处理前清空finalWetBuffer，确保不会有残留的旧数据影响当前处理，否则输出NAN值
    chorusBranch1.wetBuffer.clear();//每次处理前清空wetBuffer，确保不会有残留的旧数据影响当前处理，否则输出NAN值
    chorusBranch2.wetBuffer.clear();
    chorusBranch3.wetBuffer.clear();

	if(mDelayBufferLength <= 0 || numChannels < 2 || mSineLookUpTable.empty()){
		return;
	}

    //处理第一组声道（0和1），相位偏移为0
    processCertainChorus(buffer, 
        0.0f,
        0.0f, 
        chorusBranch1, 
        startSample, 
        numSamples);

    //相位偏移为90度（2*pi/4），右通道偏移左通道90°（two_pi/4）
    processCertainChorus(buffer, 
        currentPhaseOffsetMs,
        two_pi / 4.0f, 
        chorusBranch2,
        startSample, 
        numSamples);

    //相位偏移为-90度（-2*pi/4），右通道偏移左通道-90°（two_pi/4）
    processCertainChorus(buffer, 
        -currentPhaseOffsetMs,
        two_pi / 4.0f * 3.0f, 
        chorusBranch3,
        startSample, 
        numSamples);

    //进行加权
    //0.707是-3dB，也就是半功率点
    //而1.0^2 + 1.0^2 + 0.707^2 = 2.5
    //缩放系数：1 / sqrt(2.5) ≈ 0.632，保证总功率不变
    //最终：(1.0 * 0.632)^2 * 2 + (0.707 * 0.632)^2 ≈ 1.0，保证总幅度不变

    for(int channel = 0; channel < 2; channel++){
        juce::FloatVectorOperations::addWithMultiply(finalWetBuffer.getWritePointer(channel),
            chorusBranch1.wetBuffer.getReadPointer(channel), 0.447f, numSamples);
        juce::FloatVectorOperations::addWithMultiply(finalWetBuffer.getWritePointer(channel),
            chorusBranch2.wetBuffer.getReadPointer(channel), 0.632f, numSamples);
        juce::FloatVectorOperations::addWithMultiply(finalWetBuffer.getWritePointer(channel),
            chorusBranch3.wetBuffer.getReadPointer(channel), 0.632f, numSamples);

        //混合干湿
        auto* channelData = buffer.getWritePointer(channel, startSample);
        auto* wetData = finalWetBuffer.getWritePointer(channel);

        //currentMix * bufferSize / 4原理：index / bufferSize == mix * 0.5pi / 2pi
        juce::FloatVectorOperations::multiply(channelData, mDryTable[currentMix * bufferSize / 4], numSamples);
        juce::FloatVectorOperations::multiply(wetData, mWetTable[currentMix * bufferSize / 4], numSamples);
        juce::FloatVectorOperations::add(channelData, wetData, numSamples);
    }

}

void YOK3508Processor::processCertainChorus(
	juce::AudioBuffer<float>& buffer,
    float phaseOffsetMs,//单支路左右通道偏移毫秒数(用来确定这条支路的具体声音方位)（时间轴）
    float rightRadToLeftRad, //右声道相对于左声道的正弦波相位偏移弧度数（用来实现合唱的流动感）（信号轴）
    ChorusState &chorusState,
	int startSample,
	int numSamples)
{
    float* wetBufferDataLeft = chorusState.wetBuffer.getWritePointer(0);
    float* wetBufferDataRight = chorusState.wetBuffer.getWritePointer(1);
    //拷贝原始输入到wetBuffer中，后续对wetBuffer进行处理，最后再混回原始输入
    auto* ChannelLeftData = buffer.getReadPointer(0, startSample);
    auto* ChannelRightData = buffer.getReadPointer(1, startSample);


    std::copy(ChannelLeftData, ChannelLeftData + numSamples, wetBufferDataLeft);
    std::copy(ChannelRightData, ChannelRightData + numSamples, wetBufferDataRight);

    auto* leftDelayData = chorusState.mDelayBuffer.getWritePointer(0);
    auto* rightDelayData = chorusState.mDelayBuffer.getWritePointer(1);

	for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++){
		const float currentDepthMs = mSmoothedDepthMs.getNextValue();
		const float currentRateHz = mSmoothedRateHz.getNextValue();
        const float currentFeedback = mSmoothedFeedback.getNextValue();
        const float currentBaseDelayMs = mSmoothedBaseDelayMs.getNextValue();
        currentMix = mSmoothedMix.getNextValue();
        currentPhaseOffsetMs = mSmoothedPhaseOffsetMs.getNextValue();

        float sineValueLeft = getLinearInterpolator(mSineLookUpTable.data(), 
            static_cast<int>(mSineLookUpTable.size()), 
            chorusState.mSineTableIndex);


        float sineIndexRight = 
            getCircularBufferIndex(
                chorusState.mSineTableIndex + 
                    transformRadIntoIndexStep(rightRadToLeftRad, mSineLookUpTable.size()), 
                mSineLookUpTable.size());
        float sineValueRight = getLinearInterpolator(mSineLookUpTable.data(), 
            static_cast<int>(mSineLookUpTable.size()), 
            sineIndexRight);

		float leftDelayMs = currentBaseDelayMs + currentDepthMs * sineValueLeft + phaseOffsetMs / 2;
		float rightDelayMs = currentBaseDelayMs + currentDepthMs * sineValueRight - phaseOffsetMs / 2;

		float leftReadPosition =
			static_cast<float>(chorusState.mWritePosition) - transformMsIntoSamples(leftDelayMs, mCurrentSampleRate);
		float rightReadPosition =
			static_cast<float>(chorusState.mWritePosition) - transformMsIntoSamples(rightDelayMs, mCurrentSampleRate);

        //调节索引防止越界
        leftReadPosition = getCircularBufferIndex(
            leftReadPosition, static_cast<float>(mDelayBufferLength));
        rightReadPosition = getCircularBufferIndex(
            rightReadPosition, static_cast<float>(mDelayBufferLength));

        //获取当前输入的左右声道样本(原始样本)
		const float leftInputSample = wetBufferDataLeft[sampleIndex];
		const float rightInputSample = wetBufferDataRight[sampleIndex];

        //线性插值获取当前延迟样本（纯湿度）
        float leftWetSample = getLinearInterpolator(
			leftDelayData,
            mDelayBufferLength,
			leftReadPosition) ;

        float rightWetSample = getLinearInterpolator(
			rightDelayData,
            mDelayBufferLength,
			rightReadPosition) ;
            //这里只计算纯湿度，不要计算干湿度，干湿度等三支路都算完后再进行
            //要加入反馈度必须先读，再算反馈度，再写入延迟缓冲区
            //否则反馈度无法正确反映当前的延迟样本

        float leftWriteSample = leftInputSample + leftWetSample * currentFeedback;
        float rightWriteSample = rightInputSample + rightWetSample * currentFeedback;

        //原始样本存储，待未来使用
        //限制反馈后的样本值在-2到2之间，防止过度失真导致的爆音
		leftDelayData[chorusState.mWritePosition] = juce::jlimit(-2.0f, 2.0f, leftWriteSample);
		rightDelayData[chorusState.mWritePosition] = juce::jlimit(-2.0f, 2.0f, rightWriteSample);

        wetBufferDataLeft[sampleIndex] = leftWetSample;
        wetBufferDataRight[sampleIndex] = rightWetSample;

		chorusState.mSineTableIndex +=
			(currentRateHz / static_cast<float>(mCurrentSampleRate)) *
			static_cast<float>(mSineLookUpTable.size());

        //getCircularBufferIndex用来防止索引越界
        chorusState.mSineTableIndex = getCircularBufferIndex(
            chorusState.mSineTableIndex, static_cast<float>(mSineLookUpTable.size()));

		chorusState.mWritePosition++;
        chorusState.mWritePosition = getCircularBufferIndex(chorusState.mWritePosition, mDelayBufferLength);

	}
}



