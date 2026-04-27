#pragma once

#include <JuceHeader.h>
#include "Utils/constants.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "Utils/mathFunc.h"


class SchroederReverbEditor final : public juce::Component
{
private:

	juce::Label mTitle;
	juce::TextButton mOpenCloseButton { "Open" };

	juce::Slider decayLevelSlider;//反馈梳状滤波器反馈度
    juce::Slider diffusionLevelSlider;//扩散全通滤波器的扩散度
    juce::Slider mixLevelSlider;//干湿混合度
    juce::Slider dampHzSlider;//低通滤波器的截止频率
    juce::Slider roomSizeSlider;//房间尺寸，影响延迟时间
    juce::Slider baseDelayTimeMsSlider;//基础延迟时间，影响初始反射的时间
    juce::Slider makeUpGainSlider;//补偿增益

	juce::Label decayLevelLabel;
    juce::Label diffusionLevelLabel;
    juce::Label mixLevelLabel;
    juce::Label dampHzLabel;
    juce::Label roomSizeLabel;
    juce::Label baseDelayTimeMsLabel;
    juce::Label makeUpGainLabel;

	std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> decayLevelAttachment;
    std::unique_ptr<SliderAttachment> diffusionLevelAttachment;
    std::unique_ptr<SliderAttachment> mixLevelAttachment;
    std::unique_ptr<SliderAttachment> dampHzAttachment;
    std::unique_ptr<SliderAttachment> roomSizeAttachment;
    std::unique_ptr<SliderAttachment> baseDelayTimeMsAttachment;
    std::unique_ptr<SliderAttachment> makeUpGainAttachment;

	void bindParameters();

    juce::AudioProcessorValueTreeState& mAPVTS;

public:
	explicit SchroederReverbEditor(juce::AudioProcessorValueTreeState& apvts);
	~SchroederReverbEditor() override = default;
	void resized() override;

};

class SchroederReverbProcessor{
private:
    float currentSampleRate { defaultSampleRate };
    int currentMaximumBlockSize{bufferSize};


    bool isOpen { false };
    float decayLevel { 0.5f };//反馈梳状滤波器反馈度
    float diffusionLevel { 0.5f };//扩散全通滤波器的扩散度
    float mixLevel { 0.5f };//干湿混合度
    float dampHz { 2000.0f };//低通滤波器的截止频率
    float roomSize { 0.5f };//房间尺寸，影响延迟时间
    float baseDelayTimeMs { 50.0f };//基础延迟时间，单位为毫秒
    float makeUpGainDB { 0.0f };//补偿增益
    float makeUpGain{0.0f};//补偿增益线性值

    std::vector<float> dryTable;//干信号增益查找表，避免每次处理都进行powf计算
    std::vector<float> wetTable;//湿信号增益查找表，避免每次处理都进行powf计算

    struct combFilterSingle{//单个梳状滤波器结构体
        std::vector<float> combDelayLineBuffer;
        int writeIndex{0};
        int delaySamplesNum{0};
        int combBaseLineValue{0};//根据采样率和房间尺寸计算出的基础延迟时间对应的样本数，作为buffer大小的参考值

        void prepareToPlay(float roomSize, float sampleRate){
            delaySamplesNum = getNearestPrimeNumber(combBaseLineValue * sampleRate / defaultSampleRate * roomSize);
            writeIndex = 0;            
        }

        void setValue(bool isUptated, float sampleRate, float roomSize){
            if(isUptated)
                delaySamplesNum = getNearestPrimeNumber(combBaseLineValue * sampleRate / defaultSampleRate * roomSize);
        }

        float processSample(float inputSample, float feedback){
            float readIndex = getCircularBufferIndex(writeIndex - delaySamplesNum, combDelayLineBuffer.size());
            combDelayLineBuffer[writeIndex] = inputSample + feedback * combDelayLineBuffer[readIndex];
            writeIndex = (writeIndex + 1) % combDelayLineBuffer.size();
            
        }
    };

    struct combFilterAll{
        combFilterSingle comb1;
        combFilterSingle comb2;
        combFilterSingle comb3;
        combFilterSingle comb4;
        combFilterSingle comb5;
        combFilterSingle comb6;
        combFilterSingle comb7;
        combFilterSingle comb8;

        void setCombBufferSize(float sampleRate){

            //乘以2是为了在房间尺寸为2时也能保证足够的延迟时间，避免出现死锁问题
            comb1.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[0] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
            comb2.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[1] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
            comb3.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[2] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
            comb4.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[3] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
            comb5.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[4] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
            comb6.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[5] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
            comb7.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[6] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
            comb8.combDelayLineBuffer.resize(static_cast<int>(combBaseLineLookUp[7] * sampleRate / defaultSampleRate *2) + 1, 0.0f);
        }

        void setCombBaseLineValue(){
            comb1.combBaseLineValue = combBaseLineLookUp[0];
            comb2.combBaseLineValue = combBaseLineLookUp[1];
            comb3.combBaseLineValue = combBaseLineLookUp[2];
            comb4.combBaseLineValue = combBaseLineLookUp[3];
            comb5.combBaseLineValue = combBaseLineLookUp[4];
            comb6.combBaseLineValue = combBaseLineLookUp[5];
            comb7.combBaseLineValue = combBaseLineLookUp[6];
            comb8.combBaseLineValue = combBaseLineLookUp[7];
        }
    };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDecayLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDiffusionLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMixLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDampHz { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedRoomSize { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedBaseDelayTimeMs { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMakeUpGainDB { 1.0f };

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    juce::AudioProcessorValueTreeState& mAPVTS;

    

public:
    explicit SchroederReverbProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~SchroederReverbProcessor() = default;
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