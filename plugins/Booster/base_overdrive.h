#pragma once

#include <JuceHeader.h>
#include <array>
#include <memory>
#include <vector>
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
    juce::Slider mToneSlider;
    juce::Slider wetSlider;
    juce::Slider drySlider;

    juce::Label mDepthLabel;
    juce::Label mDriveLabel;
    juce::Label mOutputLevelLabel;
    juce::Label mToneLabel;
    juce::Label mWetLabel;
    juce::Label mDryLabel;

    std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> mDriveAttachment;
    std::unique_ptr<SliderAttachment> mOutputLevelAttachment;
    std::unique_ptr<SliderAttachment> mMixAttachment;
    std::unique_ptr<SliderAttachment> mToneAttachment;
    std::unique_ptr<SliderAttachment> mWetAttachment;
    std::unique_ptr<SliderAttachment> mDryAttachment;

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
    float mTone { 0.5f };
    float mWet { 0.5f };
    float mDry { 0.5f };

    struct LowPassFilterState {
        float cutOffFrequency { 1000.0f }; // 截止频率

        // 计算滤波器系数
        //公式：y[n] = a0 * x[n] + b0 * y[n-1]
        float a0 { 0.0f };
        float b0 { 0.0f };
        float z1 { 0.0f }; // 前一个输入样本

        void setCutOffFrequency(bool isUpdated, float tone, float sampleRate) {
            if(isUpdated){
                cutOffFrequency = tone * 10000.0f;//截止频率
                b0 = std::exp(- two_pi * cutOffFrequency / sampleRate);
                a0 = 1.0f - b0;
            }else{
                return;
            }
        }//该函数只在调节tone旋钮时调用，防止exp计算开销过大

        float processSample(float inputSample) {
            float outputSample = a0 * inputSample + b0 * z1; // 计算当前输出样本
            z1 = outputSample; // 更新前一个输出样本
            return outputSample;
        }

        void init(float tone, float sampleRate){
            cutOffFrequency = tone * 10000.0f;
            b0 = std::exp(- two_pi * cutOffFrequency / sampleRate);
            a0 = 1.0f - b0;
            z1 = 0.0f;
        }

    };//一阶低通滤波器状态

    LowPassFilterState mLowPassLeft;
    LowPassFilterState mLowPassRight;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDrive { 4.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedOutputLevel { 0.2f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedWet { 0.5f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDry { 0.5f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedTone { 0.5f };


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
