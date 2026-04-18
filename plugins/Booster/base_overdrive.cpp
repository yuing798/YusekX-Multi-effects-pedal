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
}

void baseOverdriveProcessor::syncParametersFromAPVTS()
{
    if (auto* openParameter = mAPVTS.getRawParameterValue(BaseOverdriveOpenId))
        mIsOpen = openParameter->load() >= 0.5f;
    if (auto* driveParameter = mAPVTS.getRawParameterValue(BaseOverdriveDriveId))
        mDrive = driveParameter->load();
    if (auto* outputLevelParameter = mAPVTS.getRawParameterValue(BaseOverdriveOutputLevelId))
        mOutputLevel = outputLevelParameter->load();
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

    for(int channel = 0; channel < numChannels; ++channel){

        for(int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex){
            const float currentDrive = mSmoothedDrive.getNextValue();
            const float currentOutputLevel = mSmoothedOutputLevel.getNextValue();
            auto* sampleData = buffer.getWritePointer(channel, startSample + sampleIndex);
            //简单的过载算法：将输入信号放大，然后进行软剪辑
            float inputSample = *sampleData;
            float boostedSample = inputSample * currentDrive; //放大倍数
            //软剪辑算法
            boostedSample = tanhLookUp(boostedSample) * currentOutputLevel; //使用双曲正切函数进行软剪辑
            
            *sampleData = boostedSample;
        }
    }
}



