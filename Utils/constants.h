#pragma once

//此处放置所有模块都需要用到的常量定义
static constexpr int bufferSize{ 1024 };
static constexpr float two_pi{ 2.0f * 3.14159265358979323846f };
static constexpr int numInputChannels{ 2 };
static constexpr int numOutputChannels{ 2 };
static constexpr float defaultSampleRate{ 44100.0f };

//以下声明和APVTS相关的参数ID

//延迟效果器
static constexpr const char* openParamId { "delayOpen" };
static constexpr const char* delayTimeParamId { "delayTimeMs" };
static constexpr const char* wetLevelParamId { "delayWetLevel" };
static constexpr const char* dryLevelParamId { "delayDryLevel" };
static constexpr const char* feedbackParamId { "delayFeedback" };

//颤音效果器
static constexpr const char* BaseTremoloOpenId { "baseTremoloOpen" };
static constexpr const char* BaseTremoloFreqId { "baseTremoloFreq" };
static constexpr const char* BaseTremoloDepthId { "baseTremoloDepth" };
static constexpr const char* BaseTremoloMixId { "baseTremoloMix" };
static constexpr const char* BaseTremoloDutyCycleId { "baseTremoloDutyCycle" };
static constexpr const char* BaseTremoloPeakPositionId { "baseTremoloPeakPosition" };
static constexpr const char* BaseTremoloWaveFormShapeId { "baseTremoloWaveFormShape" };


