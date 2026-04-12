#pragma once
#include <JuceHeader.h>
#include "Utils/constants.h"
#include "Utils/table.h"


class Tremolo final : public juce::Component{

private:
    enum WaveFormID{
        Sine = 1,
        Square,//方波
        Triangle,//三角波
    };

    double currentSampleRate { defaultSampleRate };
    juce::TextButton openCloseButton{"Open"};//开启关闭颤音效果的按钮
    bool isOpen{false};//颤音效果是否开启
     
    float frequency{ 5.0f };//颤音频率，默认5Hz
    float depth{ 0.5f };//颤音深度，范围0.0~1.0，默认0.5
    
    juce::Slider frequencySlider;//颤音频率滑块
    juce::Slider depthSlider;//颤音深度滑块
    juce::Slider mixSlider;//干湿比滑块
    juce::Slider dutyCycleSlider;//占空比滑块
    juce::Slider peakPositionSlider;//三角波峰值位置滑块

    juce::Label frequencyLabel;//频率标签
    juce::Label depthLabel;//深度标签
    juce::Label mixLabel;//干湿比标签
    juce::Label dutyCycleLabel;//占空比标签
    juce::Label peakPositionLabel;//三角波峰值位置标签

    juce::SmoothedValue<float, ValueSmoothingTypes::Multiplicative> 
        smoothedFrequency{ 5.0f };//平滑过渡的频率值
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> 
        smoothedDepth{ 0.5f };//平滑过渡的深度值
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> 
        smoothedMix{ 0.5f };//平滑过渡的干湿比值
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> 
        smoothedDutyCycle{ 0.5f };//平滑过渡的占空比值
    juce::SmoothedValue<float,ValueSmoothingTypes::Linear> depthGain;//仅用于方波
    
    juce::Rectangle<int> area;
    juce::Label title;
    juce::ComboBox waveFormShape;//默认波形为正弦波

    juce::SmoothedValue<float,ValueSmoothingTypes::Linear> 
        peakPosition{0.5f};//三角波峰值位置，范围0.0~1.0，默认0.5



    void resize(juce::Rectangle<int> bounds);
    void updateUI(int waveformID);

    
    void processSineTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);
    void processSquareTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);
    void processTriangleTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);

    
    bool needUpdateSineTable{false};//是否需要更新正弦波查找表的标志
    std::vector<float> sineGainTable;//正弦波增益查找表
    float sineTableIndex{ 0.0f };//正弦波查找表索引
    float lfoPhase{ 0.0f };//LFO相位
    
public:
    Tremolo();
    ~Tremolo() override = default;
    void resized() override;
    void prepareToPlay(double sampleRate);
    void processTremolo(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, int numChannels);
    
};