#pragma once
#include <JuceHeader.h>

//此处放置所有模块都需要用到的常量定义
static constexpr int bufferSize{ 1024 };
static constexpr float two_pi{ 2.0f * 3.14159265358979323846f };
static constexpr int numInputChannels{ 2 };
static constexpr int numOutputChannels{ 2 };
static constexpr float defaultSampleRate{ 44100.0f };

//APVTS参数ID

//基础延迟效果器ID
static constexpr const char* BaseDelayOpenId { "baseDelayOpen" };
static constexpr const char* BaseDelayTimeId { "baseDelayTimeMs" };
static constexpr const char* BaseDelayWetId { "baseDelayWet" };
static constexpr const char* BaseDelayDryId { "baseDelayDry" };
static constexpr const char* BaseDelayFeedbackId { "baseDelayFeedback" };

//基础颤音效果器ID
static constexpr const char* BaseTremoloOpenId { "baseTremoloOpen" };
static constexpr const char* BaseTremoloFreqId { "baseTremoloFreq" };
static constexpr const char* BaseTremoloDepthId { "baseTremoloDepth" };
static constexpr const char* BaseTremoloMixId { "baseTremoloMix" };
static constexpr const char* BaseTremoloDutyCycleId { "baseTremoloDutyCycle" };
static constexpr const char* BaseTremoloPeakPositionId { "baseTremoloPeakPosition" };
static constexpr const char* BaseTremoloWaveFormShapeId { "baseTremoloWaveFormShape" };

//正弦空间环绕效果器ID
static constexpr const char* SineSurroundOpenId = "sineSurroundOpen";
static constexpr const char* SineSurroundDepthId = "sineSurroundDepth";
static constexpr const char* SineSurroundPhaseFrequencyId = "sineSurroundPhaseFrequency";
static constexpr const char* SineSurroundDryLevelId = "sineSurroundDry";
static constexpr const char* SineSurroundWetLevelId = "sineSurroundWet";

//attachment别名
using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

//产品代号
//YOK3508:三模拟通道合唱效果器