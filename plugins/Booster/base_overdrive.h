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

    juce::Label mDepthLabel;

    std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;

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

    juce::AudioBuffer<float> finalWetBuffer;//最终的纯湿数据

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDepthMs { 4.0f };


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
