#include "eq.h"
#include "Utils/table.h"
#include "constants.h"
#include "Utils/mathFunc.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include <cstddef>
#include <memory>

baseEQProcessor::baseEQProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{

}
baseEQEditor::baseEQEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
	addAndMakeVisible(mTitle);
	mTitle.setText("base EQ", juce::dontSendNotification);

    mOpenCloseButton.setClickingTogglesState(true);
	addAndMakeVisible(mOpenCloseButton);
	mOpenCloseButton.onClick = [this]{

        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
	};

    addAndMakeVisible(mBassPassLabel);
    mBassPassLabel.setText("Bass Pass", juce::dontSendNotification);
    addAndMakeVisible(mBassPassSlider);

    addAndMakeVisible(mMiddlePassLabel);
    mMiddlePassLabel.setText("Middle Pass", juce::dontSendNotification);
    addAndMakeVisible(mMiddlePassSlider);

    addAndMakeVisible(mTreblePassLabel);
    mTreblePassLabel.setText("Treble Pass", juce::dontSendNotification);
    addAndMakeVisible(mTreblePassSlider);

    bindParameters();
}

void baseEQEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS, BaseEQOpenId, mOpenCloseButton);

    mBassPassAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseEQBassPassId, mBassPassSlider);

    mMiddlePassAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseEQMiddlePassId, mMiddlePassSlider);

    mTreblePassAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseEQTreblePassId, mTreblePassSlider);
}

void baseEQProcessor::createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &parameters){
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        BaseEQOpenId,
        "base EQ Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseEQBassPassId,
        "base EQ Bass Pass",
        juce::NormalisableRange<float>(-12.0f, 12.0f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseEQMiddlePassId,
        "base EQ Middle Pass",
        juce::NormalisableRange<float>(-12.0f, 12.0f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseEQTreblePassId,
        "base EQ Treble Pass",
        juce::NormalisableRange<float>(-12.0f, 50.0f),
        0.5f));
}

void baseEQProcessor::syncParametersFromAPVTS()
{
    if (auto* openParameter = mAPVTS.getRawParameterValue(BaseEQOpenId))
        mIsOpen = openParameter->load() >= 0.5f;
    if (auto* bassPassParameter = mAPVTS.getRawParameterValue(BaseEQBassPassId))
        mBassDBGain = bassPassParameter->load();
    if (auto* middlePassParameter = mAPVTS.getRawParameterValue(BaseEQMiddlePassId))
        mMiddleDBGain = middlePassParameter->load();
    if (auto* treblePassParameter = mAPVTS.getRawParameterValue(BaseEQTreblePassId))
        mTrebleDBGain = treblePassParameter->load();
}

void baseEQEditor::resized()
{
	mTitle.setBounds(10, 10, 100, 30);
	mOpenCloseButton.setBounds(10, 50, 100, 30);

    mBassPassLabel.setBounds(10, 90, 100, 30);
    mBassPassSlider.setBounds(120, 90, 150, 30);

    mMiddlePassLabel.setBounds(10, 130, 100, 30);
    mMiddlePassSlider.setBounds(120, 130, 150, 30);

    mTreblePassLabel.setBounds(10, 170, 100, 30);
    mTreblePassSlider.setBounds(120, 170, 150, 30);
}

void baseEQProcessor::prepareToPlay(
	double sampleRate,
	int maximumBlockSize,
	int numChannels)
{
	mCurrentSampleRate = sampleRate;


    syncParametersFromAPVTS();
	mUpdateProcessorParameters();

    mSmoothedBassLevel.reset(mCurrentSampleRate, 0.05); // 50ms的平滑时间
    mSmoothedMiddleLevel.reset(mCurrentSampleRate, 0.05);
    mSmoothedTrebleLevel.reset(mCurrentSampleRate, 0.05);
    //平滑时间只在prepareToPlay中设置

    eqFilterLeft.lowShelf.setValue(mBassDBGain, mCurrentSampleRate);
    eqFilterLeft.peakingEQ.setValue(mMiddleDBGain, mCurrentSampleRate);
    eqFilterLeft.highShelf.setValue(mTrebleDBGain, mCurrentSampleRate);
    eqFilterRight.lowShelf.setValue(mBassDBGain, mCurrentSampleRate);
    eqFilterRight.peakingEQ.setValue(mMiddleDBGain, mCurrentSampleRate);
    eqFilterRight.highShelf.setValue(mTrebleDBGain, mCurrentSampleRate);
}

void baseEQProcessor::mUpdateProcessorParameters()
{
	mSmoothedBassLevel.setTargetValue(mBassDBGain);
    mSmoothedMiddleLevel.setTargetValue(mMiddleDBGain);
    mSmoothedTrebleLevel.setTargetValue(mTrebleDBGain);


    //reset会重置isSmoothing标志位，
    // 所以在processBlock里调用mUpdateProcessorParameters时，
    // isSmoothing会返回false，这样在setCutOffFrequency函数里就不会进行exp计算，节省CPU资源


}

void baseEQProcessor::processBlock(
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

    processBaseEQ(buffer, startSample, numSamples, numChannels);
}

void baseEQProcessor::processBaseEQ(
	juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples,
    int numChannels)
{
    auto* channelDataLeft = buffer.getWritePointer(0, startSample);
    auto* channelDataRight = numChannels > 1 ? buffer.getWritePointer(1, startSample) : nullptr;
    for (int i = 0; i < numSamples; ++i) {

        float currentBassLevel = mSmoothedBassLevel.getNextValue();
        float currentMiddleLevel = mSmoothedMiddleLevel.getNextValue();
        float currentTrebleLevel = mSmoothedTrebleLevel.getNextValue();

        if(mSmoothedBassLevel.isSmoothing()){
            eqFilterLeft.lowShelf.setValue(currentBassLevel, mCurrentSampleRate);
            eqFilterRight.lowShelf.setValue(currentBassLevel, mCurrentSampleRate);

        }
        if(mSmoothedMiddleLevel.isSmoothing()){
            eqFilterLeft.peakingEQ.setValue(currentMiddleLevel, mCurrentSampleRate);
            eqFilterRight.peakingEQ.setValue(currentMiddleLevel, mCurrentSampleRate);
        }
        if(mSmoothedTrebleLevel.isSmoothing()){
            eqFilterLeft.highShelf.setValue(currentTrebleLevel, mCurrentSampleRate);
            eqFilterRight.highShelf.setValue(currentTrebleLevel, mCurrentSampleRate);
        }

        float inputSampleLeft = channelDataLeft[i];
        float processedSampleLeft = eqFilterLeft.lowShelf.processSample(inputSampleLeft);
        processedSampleLeft = eqFilterLeft.peakingEQ.processSample(processedSampleLeft);
        processedSampleLeft = eqFilterLeft.highShelf.processSample(processedSampleLeft);
        channelDataLeft[i] = processedSampleLeft;

        if (channelDataRight != nullptr) {
            float inputSampleRight = channelDataRight[i];
            float processedSampleRight = eqFilterRight.lowShelf.processSample(inputSampleRight);
            processedSampleRight = eqFilterRight.peakingEQ.processSample(processedSampleRight);
            processedSampleRight = eqFilterRight.highShelf.processSample(processedSampleRight);
            channelDataRight[i] = processedSampleRight;
        }
    }
}



