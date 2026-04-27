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

//YOK3508:三模拟通道合唱效果器
static constexpr const char* ThreeChannelsChorusOpenId = "3ChannelsChorusOpen";
static constexpr const char* ThreeChannelsChorusDepthId = "3ChannelsChorusDepth";
static constexpr const char* ThreeChannelsChorusRateId = "3ChannelsChorusRate";
static constexpr const char* ThreeChannelsChorusMixId = "3ChannelsChorusMix";
static constexpr const char* ThreeChannelsChorusFeedbackId = "3ChannelsChorusFeedback";
static constexpr const char* ThreeChannelsChorusBaseDelayId = "3ChannelsChorusBaseDelay";
static constexpr const char* ThreeChannelsChorusPhaseOffsetId = "3ChannelsChorusPhaseOffset";

//base overdrive效果器ID
static constexpr const char* BaseOverdriveOpenId = "baseOverdriveOpen";
static constexpr const char* BaseOverdriveDriveId = "baseOverdriveDrive";
static constexpr const char* BaseOverdriveOutputLevelId = "baseOverdriveOutputLevel";
static constexpr const char* BaseOverdriveMixId = "baseOverdriveMix";
static constexpr const char* BaseOverdriveToneId = "baseOverdriveTone";

//eq均衡器ID
static constexpr const char* BaseEQOpenId = "baseEQOpen";
static constexpr const char* BaseEQBassPassId = "baseEQBassPass";
static constexpr const char* BaseEQMiddlePassId = "baseEQMiddlePass";
static constexpr const char* BaseEQTreblePassId = "baseEQTreblePass";

//base compressor压缩器ID
static constexpr const char* BaseCompressorOpenId = "baseCompressorOpen";
static constexpr const char* BaseCompressorThresoldId = "baseCompressorThresold";
static constexpr const char* BaseCompressorRatioId = "baseCompressorRatio";
static constexpr const char* BaseCompressorAttackTimeId = "baseCompressorAttackTime";
static constexpr const char* BaseCompressorReleaseTimeId = "baseCompressorReleaseTime";
static constexpr const char* BaseCompressorMakeupGainId = "baseCompressorMakeupGain";

//schroeder reverb ID
static constexpr const char* SchroederReverbOpenId = "schroederReverbOpen";
static constexpr const char* SchroederReverbDecayLevelId = "schroederReverbDecayLevel";
static constexpr const char* SchroederReverbDiffusionLevelId = "schroederReverbDiffusionLevel";
static constexpr const char* SchroederReverbMixLevelId = "schroederReverbMixLevel";
static constexpr const char* SchroederReverbDampHzId = "schroederReverbDampHz";
static constexpr const char* SchroederReverbRoomSizeId = "schroederReverbRoomSize";
static constexpr const char* SchroederReverbBaseDelayTimeMsId = "schroederReverbBaseDelayTimeMs";

//combBaseLineLookUP
static constexpr const int combBaseLineLookUp[8] = {1116,1188,1277,1356,1422,1491,1557,1617};
static constexpr const int allPassBaseLineLookUp[4] = {225,341,441,556};

//attachment别名
using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
