#pragma once

#include <JuceHeader.h>
#include <memory>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"

class baseOverdriveEditor : public juce::Component
{
private:

    juce::Label mTitle;
    juce::TextButton mOpenCloseButton { "Open" };

    juce::Slider mDepthSlider;
    juce::Slider mDriveSlider;
    juce::Slider mOutputLevelSlider;
    juce::Slider mMixSlider;

    juce::Label mDepthLabel;
    juce::Label mDriveLabel;
    juce::Label mOutputLevelLabel;
    juce::Label mMixLabel;

    std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> mDriveAttachment;
    std::unique_ptr<SliderAttachment> mOutputLevelAttachment;
    std::unique_ptr<SliderAttachment> mMixAttachment;

    juce::AudioProcessorValueTreeState& mAPVTS;

    void bindParameters();

public:
    explicit baseOverdriveEditor(juce::AudioProcessorValueTreeState& apvts);
    ~baseOverdriveEditor() override = default;
    void resized() override;
};

class baseOverdriveProcessor
{
private:
    float mCurrentSampleRate{defaultSampleRate};

    bool mIsOpen { false };
    float mDrive { 4.0f };
    float mOutputLevel { 0.2f };
    float mMix { 0.5f };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDrive { 4.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedOutputLevel { 0.2f };

    juce::AudioProcessorValueTreeState& mAPVTS;

    void processBaseOverdrive(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);	

	void mUpdateProcessorParameters();

public:
    explicit baseOverdriveProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~baseOverdriveProcessor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void syncParametersFromAPVTS();


    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};
