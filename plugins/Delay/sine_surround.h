#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "Utils/constants.h"

static constexpr float maxSineDepthMs = 50.0f;
static constexpr float SineMsOffset = 300.0f;

class SineSurroundEditor final : public juce::Component
{
private:

    juce::Label mTitle;
    juce::TextButton mOpenCloseButton { "Open" };

    juce::Slider mSineDepthSlider;
    juce::Slider mPhaseFrequencySlider;
    juce::Slider mDryLevelSlider;
    juce::Slider mWetLevelSlider;

    juce::Label mSineDepthLabel;
    juce::Label mPhaseFrequencyLabel;
    juce::Label mDryLevelLabel;
    juce::Label mWetLevelLabel;

    std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> mSineDepthAttachment;
    std::unique_ptr<SliderAttachment> mPhaseFrequencyAttachment;
    std::unique_ptr<SliderAttachment> mDryLevelAttachment;
    std::unique_ptr<SliderAttachment> mWetLevelAttachment;

    juce::AudioProcessorValueTreeState& mAPVTS;

    void bindParameters();

public:
    explicit SineSurroundEditor(juce::AudioProcessorValueTreeState& apvts);
    ~SineSurroundEditor() override = default;
    void resized() override;
};

class SineSurroundProcessor
{
private:

    bool mIsOpen { false };
    float mSineDepthMs { 4.0f };
    float mPhaseFrequencyHz { 0.35f };
    float mDryLevel { 1.0f };
    float mWetLevel { 0.5f };

    juce::AudioBuffer<float> mDelayBuffer;
    std::vector<float> mSineLookUpTable;
    double mCurrentSampleRate { defaultSampleRate };
    int mDelayBufferLength { 0 };
    int mWritePosition { 0 };
    float mSineTableIndex { 0.0f };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedSineDepthMs { 4.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedPhaseFrequencyHz { 0.35f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDryLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedWetLevel { 0.5f };

    juce::AudioProcessorValueTreeState& mAPVTS;

    float mGetDelaySamples(float delayTimeMs) const;

    
    
    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

	void mUpdateProcessorParameters();

public:
    explicit SineSurroundProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~SineSurroundProcessor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void syncParametersFromAPVTS();
	void processSineSurround(
		juce::AudioBuffer<float>& buffer,
		int startSample,
		int numSamples,
		int numChannels);	

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};
