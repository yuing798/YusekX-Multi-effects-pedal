#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"

class baseEQEditor : public juce::Component
{
private:

    juce::Label mTitle;
    juce::TextButton mOpenCloseButton { "Open" };

    juce::Slider mBassPassSlider;
    juce::Slider mMiddlePassSlider;
    juce::Slider mTreblePassSlider;

    juce::Label mBassPassLabel;
    juce::Label mMiddlePassLabel;
    juce::Label mTreblePassLabel;

    std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> mBassPassAttachment;
    std::unique_ptr<SliderAttachment> mMiddlePassAttachment;
    std::unique_ptr<SliderAttachment> mTreblePassAttachment;

    juce::AudioProcessorValueTreeState& mAPVTS;

    void bindParameters();

public:
    explicit baseEQEditor(juce::AudioProcessorValueTreeState& apvts);
    ~baseEQEditor() override = default;
    void resized() override;
};

class baseEQProcessor
{
private:
    float mCurrentSampleRate{defaultSampleRate};
    float mBassDBGain{0.0f};
    float mMiddleDBGain{0.0f};
    float mTrebleDBGain{0.0f};

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedBassLevel,
        mSmoothedMiddleLevel,
        mSmoothedTrebleLevel;

    bool mIsOpen { false };

    struct LowShelfFilter {

        float f0 { 100.0f }; // 截止频率
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
            mSmoothedBassLevel;

        float A { 1.0f }; // 线性振幅倍数
        float Q { 0.707f }; // 品质因数
        float w0 { 0.0f }; // 角频率
        float alpha { 0.0f }; // 计算滤波器系数的中间变量

        float b0{0.0f};
        float b1{0.0f};
        float b2{0.0f};
        float a0{0.0f};
        float a1{0.0f};
        float a2{0.0f};

        float x1 { 0.0f }; // 前一个输入样本
        float x2 { 0.0f }; // 前两个输入样本
        float y1 { 0.0f }; // 前一个输出样本
        float y2 { 0.0f }; // 前两个输出样本


        void setValue(float bassDBGain, float sampleRate) {
            A = std::pow(10.0f, bassDBGain / 40.0f);
            w0 = two_pi * f0 / sampleRate;
            alpha = std::sin(w0) / (2.0f * Q);

            b0 = A * ((A + 1.0f) - (A - 1.0f) * std::cos(w0) + 2.0f * std::sqrt(A) * alpha);
            b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * std::cos(w0));
            b2 = A * ((A + 1.0f) - (A - 1.0f) * std::cos(w0) - 2.0f * std::sqrt(A) * alpha);
            a0 = (A + 1.0f) + (A - 1.0f) * std::cos(w0) + 2.0f * std::sqrt(A) * alpha;
            a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * std::cos(w0));
            a2 = (A + 1.0f) + (A - 1.0f) * std::cos(w0) - 2.0f * std::sqrt(A) * alpha;
        }//该函数只在调节bass旋钮时调用，防止pow和sqrt计算开销过大

        float processSample(float inputSample) {
            float outputSample = (b0 * inputSample + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0; // 计算当前输出样本
            x2 = x1; // 更新前两个输入样本
            x1 = inputSample; // 更新前一个输入样本
            y2 = y1; // 更新前两个输出样本
            y1 = outputSample; // 更新前一个输出样本

            return outputSample;
        }

    };//lowShelf状态

    struct peakingEQFilter {

        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
            mSmoothedMiddleLevel;

        float centerFrequency { 1000.0f }; // 中心频率
        float w0 { 0.0f }; // 角频率
        float A{ 1.0f }; // 线性振幅倍数
        float Q{ 0.707f }; // 品质因数
        float alpha{ 0.0f }; // 计算滤波器系数的中间变量
        float x1 { 0.0f }; // 前一个输入样本
        float x2 { 0.0f }; // 前两个输入样本
        float y1 { 0.0f }; // 前一个输出样本
        float y2 { 0.0f }; // 前两个输出样本

        float a0 { 0.0f };
        float b0 { 0.0f };
        float a1 { 0.0f };
        float b1 { 0.0f };
        float a2 { 0.0f };
        float b2 { 0.0f };

        void setValue(float middleDBGain, float sampleRate) {
            A = std::pow(10.0f, middleDBGain / 40.0f);
            w0 = two_pi * centerFrequency / sampleRate;
            alpha = std::sin(w0) / (2.0f * Q);
            b0 = 1.0f + alpha * A;
            b1 = -2.0f * std::cos(w0);
            b2 = 1.0f - alpha * A;
            a0 = 1.0f + alpha / A;
            a1 = - 2.0f * std::cos(w0);
            a2 = 1.0f - alpha / A;
        }//该函数只在调节middle旋钮时调用，防止exp计算开销过大

        float processSample(float inputSample) {
            float outputSample = (b0 * inputSample + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0; // 计算当前输出样本
            x2 = x1; // 更新前两个输入样本
            x1 = inputSample; // 更新前一个输入样本
            y2 = y1; // 更新前两个输出样本
            y1 = outputSample; // 更新前一个输出样本
            return outputSample;
        }

    };//peakingEQ状态

    struct highShelfFilter {
        float cutOffFrequency { 5000.0f }; // 截止频率
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
            mSmoothedTrebleLevel;

        float A { 1.0f }; // 线性振幅倍数
        float w0 { 0.0f }; // 角频率
        float alpha { 0.0f }; // 计算滤波器系数的中间变量
        float Q { 1.0f }; // 品质因数
        float b0{0.0f};
        float b1{0.0f};
        float b2{0.0f};
        float a0{0.0f};
        float a1{0.0f};
        float a2{0.0f};

        float x1 { 0.0f }; // 前一个输入样本
        float x2 { 0.0f }; // 前两个输入样本
        float y1 { 0.0f }; // 前一个输出样本
        float y2 { 0.0f }; // 前两个输出样本

        void setValue(float trebleDBGain, float sampleRate) {
            A = std::pow(10.0f, trebleDBGain / 40.0f);
            w0 = two_pi * cutOffFrequency / sampleRate;
            alpha = std::sin(w0) / (2.0f * Q);

            b0 = A * ((A + 1.0f) + (A - 1.0f) * std::cos(w0) + 2.0f * std::sqrt(A) * alpha);
            b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * std::cos(w0));
            b2 = A * ((A + 1.0f) + (A - 1.0f) * std::cos(w0) - 2.0f * std::sqrt(A) * alpha);
            a0 = (A + 1.0f) - (A - 1.0f) * std::cos(w0) + 2.0f * std::sqrt(A) * alpha;
            a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * std::cos(w0));
            a2 = (A + 1.0f) - (A - 1.0f) * std::cos(w0) - 2.0f * std::sqrt(A) * alpha;
        }//该函数只在调节treble旋钮时调用，防止pow和sqrt计算开销过大

        float processSample(float inputSample) {
            float outputSample = (b0 * inputSample + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0; // 计算当前输出样本
            x2 = x1; // 更新前两个输入样本
            x1 = inputSample; // 更新前一个输入样本
            y2 = y1; // 更新前两个输出样本
            y1 = outputSample; // 更新前一个输出样本

            return outputSample;
        }

    };//highShelf状态

    struct EQFilter{
        LowShelfFilter lowShelf;
        peakingEQFilter peakingEQ;
        highShelfFilter highShelf;
    };
    EQFilter eqFilterLeft;
    EQFilter eqFilterRight;

    juce::AudioProcessorValueTreeState& mAPVTS;

    void processBaseEQ(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);	

	void mUpdateProcessorParameters();

public:
    explicit baseEQProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~baseEQProcessor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void syncParametersFromAPVTS();


    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};
