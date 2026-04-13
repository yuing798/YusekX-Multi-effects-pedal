#pragma once
#include <JuceHeader.h>
#include "Utils/constants.h"
#include "Utils/table.h"

enum WaveFormID{
    Sine = 1,
    Square,//方波
    Triangle,//三角波
};


class BaseTremoloEditor : public juce::Component{

private:


    juce::TextButton mOpenCloseButton{"Open"};//开启关闭颤音效果的按钮

    
    juce::Slider mFrequencySlider;//颤音频率滑块
    juce::Slider mDepthSlider;//颤音深度滑块
    juce::Slider mMixSlider;//干湿比滑块
    juce::Slider mDutyCycleSlider;//占空比滑块
    juce::Slider mPeakPositionSlider;//三角波峰值位置滑块

    juce::Label mFrequencyLabel;//频率标签
    juce::Label mDepthLabel;//深度标签
    juce::Label mMixLabel;//干湿比标签
    juce::Label mDutyCycleLabel;//占空比标签
    juce::Label mPeakPositionLabel;//三角波峰值位置标签
    
    juce::Label mTitle;
    juce::ComboBox mWaveFormShape;//默认波形为正弦波

    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;


    std::unique_ptr<ButtonAttachment> mOpenCloseButtonAttachment;
    std::unique_ptr<SliderAttachment> mFrequencySliderAttachment;
    std::unique_ptr<SliderAttachment> mDepthSliderAttachment;
    std::unique_ptr<SliderAttachment> mMixSliderAttachment;
    std::unique_ptr<SliderAttachment> mDutyCycleSliderAttachment;
    std::unique_ptr<SliderAttachment> mPeakPositionSliderAttachment;
    std::unique_ptr<ComboBoxAttachment> mWaveFormShapeAttachment;

    juce::AudioProcessorValueTreeState& mAPVTS;
    
public:

    void updateUI(int waveformID);

    explicit BaseTremoloEditor(juce::AudioProcessorValueTreeState& apvts);
    ~BaseTremoloEditor() override = default;
    void resized() override;

    void bindParameters();
   
};

class BaseTremoloProcessor{
private:

    juce::AudioProcessorValueTreeState& mAPVTS; 

    double mCurrentSampleRate { defaultSampleRate };


    int mWaveformID { Sine };//默认波形为正弦波
    bool mIsOpen{false};//颤音效果是否开启
    float mFrequency{ 5.0f };//颤音频率，默认5Hz
    float mDepth{ 0.5f };//颤音深度，范围0.0~1.0，默认0.5
    float mMix{ 0.5f };//干湿比，范围0.0~1.0，默认0.5
    float mDutyCycle{ 0.5f };//占空比，范围0.0~1.0，默认0.5，仅用于方波
    float mPeakPosition{ 0.5f };//三角波峰值位置，范围0.0~1.0，默认0.5，仅用于三角波

    juce::SmoothedValue<float, ValueSmoothingTypes::Multiplicative> 
        mSmoothedFrequency{ 5.0f };//平滑过渡的频率值
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> 
        mSmoothedDepth{ 0.5f };//平滑过渡的深度值
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> 
        mSmoothedMix{ 0.5f };//平滑过渡的干湿比值
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> 
        mSmoothedDutyCycle{ 0.5f };//平滑过渡的占空比值
    juce::SmoothedValue<float,ValueSmoothingTypes::Linear> mSmoothedDepthGain{0.5f};//仅用于方波
    juce::SmoothedValue<float,ValueSmoothingTypes::Linear> 
        mSmoothedPeakPosition{0.5f};//三角波峰值位置，范围0.0~1.0，默认0.5
    
    std::vector<float> mSineGainTable;//正弦波增益查找表
    float mSineTableIndex{ 0.0f };//正弦波查找表索引
    float mLfoPhase{ 0.0f };//LFO相位

public:
    void processSineTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);
    void processSquareTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);
    void processTriangleTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);

    void prepareToPlay(double sampleRate);
    void processTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);

    explicit BaseTremoloProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~BaseTremoloProcessor() = default;

    void updateProcessorParameters();

    void syncParametersFromAPVTS();
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
};