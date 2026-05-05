#include "base_overdrive.h"
#include "Utils/table.h"
#include "constants.h"
#include "Utils/mathFunc.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_dsp/juce_dsp.h"
#include <memory>
#include <vector>

baseOverdriveProcessor::baseOverdriveProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{

}
baseOverdriveEditor::baseOverdriveEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
	addAndMakeVisible(mTitle);
	mTitle.setText("base overdrive", juce::dontSendNotification);

    mOpenCloseButton.setClickingTogglesState(true);
	addAndMakeVisible(mOpenCloseButton);
	mOpenCloseButton.onClick = [this]{

        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
	};

    addAndMakeVisible(mDriveLabel);
    mDriveLabel.setText("Drive", juce::dontSendNotification);
    addAndMakeVisible(mDriveSlider);

    addAndMakeVisible(mOutputLevelLabel);
    mOutputLevelLabel.setText("Output Level", juce::dontSendNotification);
    addAndMakeVisible(mOutputLevelSlider);

    addAndMakeVisible(mToneLabel);
    mToneLabel.setText("Tone", juce::dontSendNotification);
    addAndMakeVisible(mToneSlider);

    addAndMakeVisible(mWetLabel);
    mWetLabel.setText("Wet", juce::dontSendNotification);
    addAndMakeVisible(wetSlider);

    addAndMakeVisible(mDryLabel);
    mDryLabel.setText("Dry", juce::dontSendNotification);
    addAndMakeVisible(drySlider);

    bindParameters();
}

void baseOverdriveEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS, BaseOverdriveOpenId, mOpenCloseButton);

    mDriveAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseOverdriveDriveId, mDriveSlider);

    mOutputLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseOverdriveOutputLevelId, mOutputLevelSlider);

    mWetAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseOverdriveWetId, wetSlider);

    mDryAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseOverdriveDryId, drySlider);

    mToneAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseOverdriveToneId, mToneSlider);
}

void baseOverdriveProcessor::createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &parameters){
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        BaseOverdriveOpenId,
        "base Overdrive Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseOverdriveDriveId,
        "base Overdrive Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f),
        4.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseOverdriveOutputLevelId,
        "base Overdrive Output Level",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseOverdriveWetId,
        "base Overdrive Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseOverdriveDryId,
        "base Overdrive Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        BaseOverdriveToneId,
        "base Overdrive Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f));
}

void baseOverdriveProcessor::syncParametersFromAPVTS()
{
    if (auto* openParameter = mAPVTS.getRawParameterValue(BaseOverdriveOpenId))
        mIsOpen = openParameter->load() >= 0.5f;
    if (auto* driveParameter = mAPVTS.getRawParameterValue(BaseOverdriveDriveId))
        mDrive = driveParameter->load();
    if (auto* outputLevelParameter = mAPVTS.getRawParameterValue(BaseOverdriveOutputLevelId))
        mOutputLevel = outputLevelParameter->load();
    if (auto* wetParameter = mAPVTS.getRawParameterValue(BaseOverdriveWetId))
        mWet = wetParameter->load();
    if (auto* dryParameter = mAPVTS.getRawParameterValue(BaseOverdriveDryId))
        mDry = dryParameter->load();
    if (auto* toneParameter = mAPVTS.getRawParameterValue(BaseOverdriveToneId))
        mTone = toneParameter->load();
}

void baseOverdriveEditor::resized()
{
	mTitle.setBounds(10, 10, 100, 30);
	mOpenCloseButton.setBounds(10, 50, 100, 30);

    mDriveLabel.setBounds(10, 90, 100, 30);
    mDriveSlider.setBounds(120, 90, 150, 30);

    mOutputLevelLabel.setBounds(10, 130, 100, 30);
    mOutputLevelSlider.setBounds(120, 130, 150, 30);

    mWetLabel.setBounds(10, 170, 100, 30);
    wetSlider.setBounds(120, 170, 150, 30);

    mDryLabel.setBounds(10, 210, 100, 30);
    drySlider.setBounds(120, 210, 150, 30);

    mToneLabel.setBounds(10, 250, 100, 30);
    mToneSlider.setBounds(120, 250, 150, 30);
}

void baseOverdriveProcessor::prepareToPlay(
	double sampleRate,
	int maximumBlockSize,
	int numChannels)
{
	mCurrentSampleRate = sampleRate;


    syncParametersFromAPVTS();
	mUpdateProcessorParameters();

    mLowPassLeft.init(mTone, mCurrentSampleRate); // 初始化低通滤波器状态
    mLowPassRight.init(mTone, mCurrentSampleRate);

    mSmoothedDrive.reset(mCurrentSampleRate, 0.05); // 50ms的平滑时间
    mSmoothedOutputLevel.reset(mCurrentSampleRate, 0.05);
    mSmoothedWet.reset(mCurrentSampleRate, 0.05);
    mSmoothedDry.reset(mCurrentSampleRate, 0.05);
    mSmoothedTone.reset(mCurrentSampleRate, 0.05);
    //平滑时间只在prepareToPlay中设置
}

void baseOverdriveProcessor::mUpdateProcessorParameters()
{



	mSmoothedDrive.setTargetValue(mDrive);
    mSmoothedOutputLevel.setTargetValue(mOutputLevel);
    mSmoothedWet.setTargetValue(mWet);
    mSmoothedDry.setTargetValue(mDry);
    mSmoothedTone.setTargetValue(mTone);


    //reset会重置isSmoothing标志位，
    // 所以在processBlock里调用mUpdateProcessorParameters时，
    // isSmoothing会返回false，这样在setCutOffFrequency函数里就不会进行exp计算，节省CPU资源


}

void baseOverdriveProcessor::processBlock(
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

    processBaseOverdrive(buffer, startSample, numSamples, numChannels);
}

void baseOverdriveProcessor::processBaseOverdrive(
	juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples,
    int numChannels)
{
    auto* channelDataLeft = buffer.getWritePointer(0, startSample);
    auto* channelDataRight = numChannels > 1 ? buffer.getWritePointer(1, startSample) : nullptr;

    for(int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex){

        float currentDrive = mSmoothedDrive.getNextValue();
        float currentOutputLevel = mSmoothedOutputLevel.getNextValue();
        float currentWet = mSmoothedWet.getNextValue();
        float currentDry = mSmoothedDry.getNextValue();
        float currentTone = mSmoothedTone.getNextValue();

        mLowPassLeft.setCutOffFrequency(mSmoothedTone.isSmoothing(), currentTone, mCurrentSampleRate);


        //信号放大
        float inputSampleLeft = channelDataLeft[sampleIndex] * currentDrive;

        //过载失真
        std::vector<float> boostBufferLeft = overSamplingStateLeft.processUpSamplingMultiPhase(inputSampleLeft);
        for(size_t index = 0; index < boostBufferLeft.size(); index++){
            tanhApproximate(boostBufferLeft[index]);
        }
        inputSampleLeft = overSamplingStateLeft.processDownSamplingMultiPhase(boostBufferLeft);

        //一阶低通滤波器处理
        inputSampleLeft = mLowPassLeft.processSample(inputSampleLeft);


        //干湿混合
        channelDataLeft[sampleIndex] = 
            inputSampleLeft * currentWet + channelDataLeft[sampleIndex] * currentDry;

        //增益补偿
        channelDataLeft[sampleIndex] *= currentOutputLevel;

        if(channelDataRight != nullptr){
            mLowPassRight.setCutOffFrequency(mSmoothedTone.isSmoothing(), currentTone, mCurrentSampleRate);
            float inputSampleRight = channelDataRight[sampleIndex] * currentDrive;

            std::vector<float> boostBufferRight = overSamplingStateRight.processUpSamplingMultiPhase(inputSampleRight);
            for(size_t index = 0; index < boostBufferRight.size(); index++){
                tanhApproximate(boostBufferRight[index]);
            }

            inputSampleRight = overSamplingStateRight.processDownSamplingMultiPhase(boostBufferRight);

            inputSampleRight = mLowPassRight.processSample(inputSampleRight);
            channelDataRight[sampleIndex] =
                inputSampleRight * currentWet + channelDataRight[sampleIndex] * currentDry;

            channelDataRight[sampleIndex] *= currentOutputLevel;
        }
    }
}



