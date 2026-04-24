#pragma once

#include <JuceHeader.h>
#include <memory>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"


//三模拟通道合唱效果器
class BaseCompressorEditor : public juce::Component
{
private:

    juce::Label mTitle;
    juce::TextButton mOpenCloseButton { "Open" };

    juce::Slider thresoldDBSlider;
    juce::Slider ratioSlider;
    juce::Slider attackTimeSlider;
    juce::Slider releaseTimeSlider;
    juce::Slider makeupGainSlider;

    juce::Label thresoldDBLabel;
    juce::Label ratioLabel;
    juce::Label attackTimeLabel;
    juce::Label releaseTimeLabel;
    juce::Label makeupGainLabel;

    std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> thresoldDBAttachment;
    std::unique_ptr<SliderAttachment> ratioAttachment;
    std::unique_ptr<SliderAttachment> attackTimeAttachment;
    std::unique_ptr<SliderAttachment> releaseTimeAttachment;
    std::unique_ptr<SliderAttachment> makeupGainAttachment;

    juce::AudioProcessorValueTreeState& mAPVTS;

    void bindParameters();

public:
    explicit BaseCompressorEditor(juce::AudioProcessorValueTreeState& apvts);
    ~BaseCompressorEditor() override = default;
    void resized() override;
};

class BaseCompressorProcessor
{
private:

    float mCurrentSampleRate { defaultSampleRate };

    bool mIsOpen { false };
    float thresoldDB { -20.0f };//阈值
    float ratio { 4.0f };//压缩比
    float attackTimeMs { 10.0f };//启动时间
    float releaseTimeMs { 100.0f };//释放时间
    float makeupGainDB { 0.0f };//补偿增益

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedThresoldDB { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedRatio { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedAttackTimeMs { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedReleaseTimeMs { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMakeupGainDB { 1.0f };

    juce::AudioProcessorValueTreeState& mAPVTS;

    void processBlock(
        juce::AudioBuffer<float>& buffer,
		int startSample,
		int numSamples,
        int numChannels
    );	


	void updateProcessorParameters();

    

public:
    explicit BaseCompressorProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~BaseCompressorProcessor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void syncParametersFromAPVTS();


    void processCompressor(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};
