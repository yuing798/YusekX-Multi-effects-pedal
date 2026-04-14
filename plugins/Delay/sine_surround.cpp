#include "sine_surround.h"
#include "Utils/table.h"
#include "constants.h"
#include "Utils/mathFunc.h"
SineSurroundProcessor::SineSurroundProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{

}
SineSurroundEditor::SineSurroundEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
	addAndMakeVisible(mTitle);
	mTitle.setText("Sine Surround", juce::dontSendNotification);

	addAndMakeVisible(mOpenCloseButton);
	mOpenCloseButton.onClick = [this]{
        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
	};

	addAndMakeVisible(mSineDepthLabel);
	mSineDepthLabel.setText("Sine Depth", juce::dontSendNotification);
	addAndMakeVisible(mSineDepthSlider);

	addAndMakeVisible(mPhaseFrequencyLabel);
	mPhaseFrequencyLabel.setText("Phase Frequency", juce::dontSendNotification);
	addAndMakeVisible(mPhaseFrequencySlider);

    addAndMakeVisible(mDryLevelLabel);
    mDryLevelLabel.setText("Dry", juce::dontSendNotification);
    addAndMakeVisible(mDryLevelSlider);

    addAndMakeVisible(mWetLevelLabel);
    mWetLevelLabel.setText("Wet", juce::dontSendNotification);
    addAndMakeVisible(mWetLevelSlider);

}

void SineSurroundEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS, SineSurroundOpenId, mOpenCloseButton);

    mSineDepthAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, SineSurroundDepthId, mSineDepthSlider);

    mPhaseFrequencyAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, SineSurroundPhaseFrequencyId, mPhaseFrequencySlider);

    mDryLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, SineSurroundDryLevelId, mDryLevelSlider);

    mWetLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, SineSurroundWetLevelId, mWetLevelSlider);
}

void SineSurroundProcessor::createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &parameters){
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        SineSurroundOpenId,
        "Sine Surround Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SineSurroundDepthId, 1 },
        "Sine Depth",
        juce::NormalisableRange<float>(-maxSineDepthMs, maxSineDepthMs, 0.1f),
        4.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SineSurroundPhaseFrequencyId, 1 },
        "Phase Frequency",
        juce::NormalisableRange<float>(0.01f, 5.0f, 0.01f),
        0.35f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SineSurroundDryLevelId, 1 },
        "Dry Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SineSurroundWetLevelId, 1 },
        "Wet Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));
}

void SineSurroundProcessor::syncParametersFromAPVTS()
{
    if (auto* openParameter = mAPVTS.getRawParameterValue(SineSurroundOpenId))
        isOpen = openParameter->load() >= 0.5f;

    if (auto* sineDepthParameter = mAPVTS.getRawParameterValue(SineSurroundDepthId))
        sineDepthMs = sineDepthParameter->load();

    if (auto* phaseFrequencyParameter = mAPVTS.getRawParameterValue(SineSurroundPhaseFrequencyId))
        phaseFrequencyHz = phaseFrequencyParameter->load();

    if (auto* dryLevelParameter = mAPVTS.getRawParameterValue(SineSurroundDryLevelId))
        dryLevel = dryLevelParameter->load();

    if (auto* wetLevelParameter = mAPVTS.getRawParameterValue(SineSurroundWetLevelId))
        wetLevel = wetLevelParameter->load();
}

void SineSurroundEditor::resized()
{
	mTitle.setBounds(10, 10, 100, 30);
	mOpenCloseButton.setBounds(10, 50, 100, 30);

	mSineDepthSlider.setBounds(220,10,150,30);
	mSineDepthLabel.setBounds(130,10,80,30);

	mPhaseFrequencySlider.setBounds(220,50,150,30);
	mPhaseFrequencyLabel.setBounds(130,50,110,30);

    mDryLevelSlider.setBounds(60, 90, 150, 30);
    mDryLevelLabel.setBounds(10, 90, 50, 30);

    mWetLevelSlider.setBounds(60, 130, 150, 30);
    mWetLevelLabel.setBounds(10, 130, 50, 30);
}

void SineSurroundProcessor::prepareToPlay(
	double sampleRate,
	int maximumBlockSize,
	int numChannels)
{
	mCurrentSampleRate = sampleRate;

	const auto maxDelaySamples = static_cast<int>(std::ceil(
		((SineMsOffset + maxSineDepthMs) / 1000.0f) *
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

	mSmoothedSineDepthMs.reset(sampleRate, 0.02);
	mSmoothedPhaseFrequencyHz.reset(sampleRate, 0.02);
    mSmoothedWetLevel.reset(sampleRate, 0.02);
    mSmoothedDryLevel.reset(sampleRate, 0.02);

	mUpdateProcessorParameters();
    syncParametersFromAPVTS();
}

void SineSurroundProcessor::processSineSurround(
	juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples,
	int numChannels)
{
	if(!isOpen){
		return;
	}
    mUpdateProcessorParameters();
    syncParametersFromAPVTS();

	processBlock(buffer, startSample, numSamples, numChannels);
}

void SineSurroundProcessor::mUpdateProcessorParameters()
{
	mSmoothedSineDepthMs.setTargetValue(sineDepthMs);
	mSmoothedPhaseFrequencyHz.setTargetValue(phaseFrequencyHz);
	mSmoothedDryLevel.setTargetValue(dryLevel);
	mSmoothedWetLevel.setTargetValue(wetLevel);
}

void SineSurroundProcessor::processBlock(
	juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples,
	int numChannels)
{
	if(mDelayBufferLength <= 0 || numChannels < 2 || mSineLookUpTable.empty()){
		return;
	}


	auto* leftChannelData = buffer.getWritePointer(0, startSample);
	auto* rightChannelData = buffer.getWritePointer(1, startSample);
	auto* leftDelayData = mDelayBuffer.getWritePointer(0);
	auto* rightDelayData = mDelayBuffer.getWritePointer(1);

	for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++){
		const float currentDepthMs = mSmoothedSineDepthMs.getNextValue();
		const float currentPhaseFrequencyHz = mSmoothedPhaseFrequencyHz.getNextValue();

		const int sineIndex = static_cast<int>(mSineTableIndex);
		const float sineValue = mSineLookUpTable[sineIndex];

		float leftDelayMs = SineMsOffset;
		float rightDelayMs = SineMsOffset + currentDepthMs * sineValue;

		float leftReadPosition =
			static_cast<float>(mWritePosition) - mGetDelaySamples(leftDelayMs);
		float rightReadPosition =
			static_cast<float>(mWritePosition) - mGetDelaySamples(rightDelayMs);

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
			leftReadPosition) * mSmoothedWetLevel.getCurrentValue() +
            leftInputSample * mSmoothedDryLevel.getCurrentValue();
		rightChannelData[sampleIndex] = getLinearInterpolator(
			rightDelayData,
            mDelayBufferLength,
			rightReadPosition) * mSmoothedWetLevel.getCurrentValue() +
            rightInputSample * mSmoothedDryLevel.getCurrentValue();

		mSineTableIndex +=
			(currentPhaseFrequencyHz / static_cast<float>(mCurrentSampleRate)) *
			static_cast<float>(mSineLookUpTable.size());

        mSineTableIndex = getCircularBufferIndex(
            mSineTableIndex, static_cast<float>(mSineLookUpTable.size()));

		mWritePosition++;
        mWritePosition = getCircularBufferIndex(mWritePosition, mDelayBufferLength);

	}
}

//返回实际延迟样本数目
float SineSurroundProcessor::mGetDelaySamples(float delayTimeMs) const
{
	return juce::jlimit(
		1.0f,
		static_cast<float>(mDelayBufferLength - 1),
		(delayTimeMs / 1000.0f) * static_cast<float>(mCurrentSampleRate));
}
