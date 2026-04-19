#include "base_overdrive.h"
#include "Utils/table.h"
#include "constants.h"
#include "Utils/mathFunc.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <memory>

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

    addAndMakeVisible(mMixLabel);
    mMixLabel.setText("Mix", juce::dontSendNotification);
    addAndMakeVisible(mMixSlider);

    addAndMakeVisible(mToneLabel);
    mToneLabel.setText("Tone", juce::dontSendNotification);
    addAndMakeVisible(mToneSlider);

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

    mMixAttachment = std::make_unique<SliderAttachment>(
        mAPVTS, BaseOverdriveMixId, mMixSlider);

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
        BaseOverdriveMixId,
        "base Overdrive Mix",
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
    if (auto* mixParameter = mAPVTS.getRawParameterValue(BaseOverdriveMixId))
        mMix = mixParameter->load();
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

    mMixLabel.setBounds(10, 170, 100, 30);
    mMixSlider.setBounds(120, 170, 150, 30);

    mToneLabel.setBounds(10, 210, 100, 30);
    mToneSlider.setBounds(120, 210, 150, 30);
}

void baseOverdriveProcessor::prepareToPlay(
	double sampleRate,
	int maximumBlockSize,
	int numChannels)
{
	mCurrentSampleRate = sampleRate;


    syncParametersFromAPVTS();
	mUpdateProcessorParameters();

}

void baseOverdriveProcessor::mUpdateProcessorParameters()
{
	mSmoothedDrive.setTargetValue(mDrive);
    mSmoothedOutputLevel.setTargetValue(mOutputLevel);
    mSmoothedMix.setTargetValue(mMix);
    mSmoothedTone.setTargetValue(mTone);

    mSmoothedDrive.reset(mCurrentSampleRate, 0.05); // 50ms的平滑时间
    mSmoothedOutputLevel.reset(mCurrentSampleRate, 0.05);
    mSmoothedMix.reset(mCurrentSampleRate, 0.05);
    mSmoothedTone.reset(mCurrentSampleRate, 0.05);

}

void baseOverdriveProcessor::processBlock(
	juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples,
	int numChannels,
    const std::vector<float>& wetTable,
    const std::vector<float>& dryTable)
{
    syncParametersFromAPVTS();
    mUpdateProcessorParameters();

	if(!mIsOpen){
		return;
	}

    processBaseOverdrive(buffer, startSample, numSamples, numChannels, wetTable, dryTable);
}

void baseOverdriveProcessor::processBaseOverdrive(
	juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples,
    int numChannels,
    const std::vector<float>& wetTable,
    const std::vector<float>& dryTable)
{
    for(int channel = 0; channel < numChannels; ++channel){
        auto* channelData = buffer.getWritePointer(channel, startSample);
        
        // 为临时缓冲区分配内存
        std::vector<float> wetBuffer(numSamples);
        
        for(int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex){
            const float currentDrive = mSmoothedDrive.getNextValue();
            const float currentOutputLevel = mSmoothedOutputLevel.getNextValue();
            const float currentMix = mSmoothedMix.getNextValue();
            
            // 获取原始输入样本
            const float inputSample = channelData[sampleIndex];
            
            // 简单的过载算法：将输入信号放大，然后进行软剪辑
            float boostedSample = inputSample * currentDrive; // 放大倍数
            // 软剪辑算法
            boostedSample = tanhLookUp(boostedSample) * currentOutputLevel;
            
            // 存储处理后的湿信号
            wetBuffer[sampleIndex] = boostedSample;
            
            // 干湿混合
            const float dryGain = dryTable[static_cast<size_t>(currentMix * bufferSize / 4)];
            const float wetGain = wetTable[static_cast<size_t>(currentMix * bufferSize / 4)];
            
            channelData[sampleIndex] = inputSample * dryGain + wetBuffer[sampleIndex] * wetGain;
        }
    }
}



