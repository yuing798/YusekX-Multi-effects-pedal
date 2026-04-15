#pragma once

#include <JuceHeader.h>
#include <memory>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"


//三模拟通道合唱效果器
class YOK3508Editor : public juce::Component
{
private:

    juce::Label mTitle;
    juce::TextButton mOpenCloseButton { "Open" };

    juce::Slider mDepthSlider;
    juce::Slider mMixSlider;
    juce::Slider mRateSlider;
    juce::Slider mFeedbackSlider;
    juce::Slider mBaseDelaySlider;

    juce::Label mDepthLabel;
    juce::Label mMixLabel;
    juce::Label mRateLabel;
    juce::Label mFeedbackLabel;
    juce::Label mBaseDelayLabel;


    std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> mDepthAttachment;
    std::unique_ptr<SliderAttachment> mMixAttachment;
    std::unique_ptr<SliderAttachment> mRateAttachment;
    std::unique_ptr<SliderAttachment> mFeedbackAttachment;
    std::unique_ptr<SliderAttachment> mBaseDelayAttachment;


    juce::AudioProcessorValueTreeState& mAPVTS;

    void bindParameters();

public:
    explicit YOK3508Editor(juce::AudioProcessorValueTreeState& apvts);
    ~YOK3508Editor() override = default;
    void resized() override;
};

class YOK3508Processor
{
private:
    const float maxSineDepthMs = 8.0f;

    bool mIsOpen { false };
    float mDepthMs { 4.0f };
    float mRateHz { 0.35f };
    float mMix { 0.5f };
    float mFeedback { 0.0f };
    float mBaseDelayMs { 4.0f };

    
    std::vector<float> mSineLookUpTable;
    double mCurrentSampleRate { defaultSampleRate };
    int mDelayBufferLength { 0 };
    
    
    juce::AudioBuffer<float> wetBuffer;

    struct ChorusState
    {
        juce::AudioBuffer<float> mDelayBuffer;
        int mWritePosition { 0 };
        float mSineTableIndex { 0.0f };
    };//每个合唱支路都有一个ChorusState，里面包含一个延迟缓冲区和写入位置以及正弦表索引

    ChorusState chorusBranch1;
    ChorusState chorusBranch2;
    ChorusState chorusBranch3;

    std::vector<float> mWetTable;
    std::vector<float> mDryTable;

    juce::AudioBuffer<float> finalWetBuffer;//最终的纯湿数据

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDepthMs { 4.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMix { 0.5f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedRateHz { 0.35f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedFeedback { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedBaseDelayMs { 4.0f };


    juce::AudioProcessorValueTreeState& mAPVTS;

    void processCertainChorus(
        juce::AudioBuffer<float>& buffer,
        float* wetBufferDataLeft,
        float* wetBufferDataRight,
        float phaseOffsetMs,//单支路左右通道偏移毫秒数(用来确定这条支路的具体声音方位)（时间轴）
        float rightRadToLeftRad, //右声道相对于左声道的正弦波相位偏移弧度数（用来实现合唱的流动感）（信号轴）
        ChorusState &chorusState,
		int startSample,
		int numSamples);	


	void mUpdateProcessorParameters();

    

public:
    explicit YOK3508Processor(juce::AudioProcessorValueTreeState& apvts);
    ~YOK3508Processor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void syncParametersFromAPVTS();


    void processThreeChannelsChorus(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};
