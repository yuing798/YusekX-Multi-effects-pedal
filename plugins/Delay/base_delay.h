#pragma once

#include <JuceHeader.h>
#include <memory>
#include "Utils/constants.h"

class BaseDelayEditor final : public juce::Component
{
private:

	juce::Label mTitle;
	juce::TextButton mOpenCloseButton { "Open" };

	juce::Slider mDelayTimeSlider;
	juce::Slider mWetLevelSlider;
	juce::Slider mDryLevelSlider;
    juce::Slider mFeedbackSlider;

	juce::Label mDelayTimeLabel;
	juce::Label mWetLevelLabel;
	juce::Label mDryLevelLabel;     
    juce::Label mFeedbackLabel;

	std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
	std::unique_ptr<SliderAttachment> mDelayTimeAttachment;
	std::unique_ptr<SliderAttachment> mWetLevelAttachment;
	std::unique_ptr<SliderAttachment> mDryLevelAttachment;
	std::unique_ptr<SliderAttachment> mFeedbackAttachment;

	
	void bindParameters();

    juce::AudioProcessorValueTreeState& mAPVTS;

public:
	explicit BaseDelayEditor(juce::AudioProcessorValueTreeState& apvts);
	~BaseDelayEditor() override = default;
	void resized() override;

};

class BaseDelayProcessor{
private:
    static constexpr float baseDelaymaxDelayTimeMs { 500.0f };


    bool isOpen { false };
    float delayTimeMs { 350.0f };

    //对于非相干信号，如果要保持听感响度一致，需要wet^2 + dry^2 = 1
    float wetLevel { 0.35f };
    float dryLevel { 1.0f };
    float feedback { 0.5f };

    juce::AudioBuffer<float> mDelayBuffer;
    double mCurrentSampleRate {defaultSampleRate };
    int mDelayBufferLength { 0 };
    int mWritePosition { 0 };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDelayTimeMs { 350.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedWetLevel { 0.35f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDryLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedFeedback { 0.5f };

    float getDelaySamples(float delayTimeMs) const;

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    juce::AudioProcessorValueTreeState& mAPVTS;

    

public:
    explicit BaseDelayProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~BaseDelayProcessor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void updateProcessorParameters();

    void processDelay(
		juce::AudioBuffer<float>& buffer,
		int startSample,
		int numSamples,
		int numChannels);

    void syncParametersFromAPVTS();

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};
