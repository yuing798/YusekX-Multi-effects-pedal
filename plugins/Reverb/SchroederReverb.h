#pragma once

#include <JuceHeader.h>
#include <vector>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_dsp/juce_dsp.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "Utils/mathFunc.h"
#include "dspFilters.h"

class SchroederReverbEditor final : public juce::Component
{
private:

	juce::Label mTitle;
	juce::TextButton mOpenCloseButton { "Open" };

	juce::Slider decayLevelSlider;//反馈梳状滤波器反馈度
    juce::Slider diffusionLevelSlider;//扩散全通滤波器的扩散度
    juce::Slider dryLevelSlider;
    juce::Slider wetLevelSlider;
    juce::Slider dampLevelSlider;//低通滤波器的截止频率
    juce::Slider roomSizeSlider;//房间尺寸，影响延迟时间
    juce::Slider baseDelayTimeMsSlider;//基础延迟时间，影响初始反射的时间
    juce::Slider makeUpGainSlider;//补偿增益

	juce::Label decayLevelLabel;
    juce::Label diffusionLevelLabel;
    juce::Label dryLevelLabel;
    juce::Label wetLevelLabel;
    juce::Label dampLevelLabel;
    juce::Label roomSizeLabel;
    juce::Label baseDelayTimeMsLabel;
    juce::Label makeUpGainLabel;

	std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> decayLevelAttachment;
    std::unique_ptr<SliderAttachment> diffusionLevelAttachment;
    std::unique_ptr<SliderAttachment> dryLevelAttachment;
    std::unique_ptr<SliderAttachment> wetLevelAttachment;
    std::unique_ptr<SliderAttachment> dampLevelAttachment;
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
    float dryLevel { 0.5f };
    float wetLevel { 0.5f };
    float dampLevel { 0.5f };//低通滤波器系数
    float roomSize { 0.5f };//房间尺寸，影响延迟时间
    float baseDelayTimeMs { 50.0f };//基础延迟时间，单位为毫秒
    float makeUpGainDB { 0.0f };//补偿增益
    float makeUpGain{0.0f};//补偿增益线性值

    std::vector<preDelay> preDelays{2};

    struct combFilterSingle{//单个反馈梳状滤波器结构体
        //y[n] = x[n] + decay * y[n - delaySamples]
        std::vector<float> combDelayLineBuffer;
    
        int writeIndex{0};//写指针八个滤波器共用同一个
        float delaySamplesNum{0.0f};
        int combDelayLineValue{0};//根据采样率和房间尺寸计算出的基础延迟时间对应的样本数，作为buffer大小的参考值

        lowPassFilter dampFilter;//每个梳状滤波器内置一个低通滤波器，用于模拟高频衰减

        void prepareToPlay(float sampleRate, float roomSize, float dampLevel){
            delaySamplesNum = getNearestPrimeNumber(combDelayLineValue * sampleRate / defaultSampleRate * roomSize);
            writeIndex = 0;
            dampFilter.prepareToPlay(dampLevel);
        }
        

        void setValue(float sampleRate, float roomSize, float dampLevel){

            delaySamplesNum = getNearestPrimeNumber(combDelayLineValue * sampleRate / defaultSampleRate * roomSize);
            dampFilter.setValue(dampLevel);
        }

        float processSample(float inputSample, float decay){
            float readIndex = getCircularBufferIndex(writeIndex - delaySamplesNum, combDelayLineBuffer.size());
            float readSample = getLinearInterpolator(combDelayLineBuffer.data(), static_cast<int>(combDelayLineBuffer.size()), readIndex);
            readSample = dampFilter.processSample(readSample);
            combDelayLineBuffer[writeIndex] = inputSample + decay * readSample;
            return combDelayLineBuffer[writeIndex];
        }
    };

    struct combFilterAll{

        std::vector<combFilterSingle> combFilters{8};

        int combBufferSize{0};

        void setCombBufferSize(float sampleRate){

            combBufferSize = static_cast<int>(combDelayLineLookUp[7] * sampleRate / defaultSampleRate * 2.1) + 1;
            //乘以2是为了在房间尺寸为2时也能保证足够的延迟时间，避免出现死锁问题
            //所有梳状滤波器共用同一个缓冲区，大小根据最长的延迟时间来设置，确保在任何房间尺寸下都能正常工作
            for(int i = 0; i < 8; i++){
                combFilters[i].combDelayLineBuffer.resize(combBufferSize, 0.0f);
            }
        }

        void setCombDelayLineValue(){
            for(int i = 0; i < 8; i++){
                combFilters[i].combDelayLineValue = combDelayLineLookUp[i];
            }
        }

        void prepareToPlay(float sampleRate, float roomSize, float dampHz){
            setCombDelayLineValue();
            setCombBufferSize(sampleRate);
            for(int i = 0; i < 8; i++){
                combFilters[i].prepareToPlay(sampleRate, roomSize, dampHz);
            }
        }

        void setValue(float sampleRate, float roomSize, float dampLevel){
            for(int i = 0; i < 8; i++){
                combFilters[i].setValue(sampleRate, roomSize, dampLevel);
            }
        }//这个在roomSize改变时调用，更新每个梳状滤波器的delaySamplesNum值

        float processSample(float inputSample, float decay){
            //八个梳状滤波器并行处理输入信号，输出相加
            float outputSample = 0.0f;
            for(int i = 0; i < 8; i++){
                outputSample += combFilters[i].processSample(inputSample, decay);
                combFilters[i].writeIndex = getCircularBufferIndex(combFilters[i].writeIndex + 1, combBufferSize);
            }

            return outputSample / 8.0f;//平均输出，避免增益过大
        }
    };

    std::vector<combFilterAll> combFilterAlls{2};//左右声道各一个

    struct allPassFilterAll{
        std::vector<allPassFilterSingle> allPassFilters{4};

        int allPassBufferSize{0};
        int writeIndex{0};//四个滤波器共用一个写指针，确保它们的写位置始终保持一致

        void setAllPassBufferSize(float sampleRate){
            allPassBufferSize = static_cast<int>(allPassDelayLineLookUp[3] * sampleRate / defaultSampleRate * 2.1) + 1;
            //乘以2是为了在房间尺寸为2时也能保证足够的延迟时间，避免出现死锁问题
            //所有全通滤波器共用同一个缓冲区，大小根据最长的延迟时间来设置，确保在任何房间尺寸下都能正常工作
            for(int i = 0; i < 4; i++){
                allPassFilters[i].allPassDelayLineBuffer.resize(allPassBufferSize, 0.0f);
            }
        }

        void setAllPassDelayLineValue(){
            for(int i = 0; i < 4; i++){
                allPassFilters[i].allPassDelayLineValue = allPassDelayLineLookUp[i];
            }
        }

        void prepareToPlay(float sampleRate, float roomSize){
            setAllPassDelayLineValue();
            setAllPassBufferSize(sampleRate);
            for(int i = 0; i < 4; i++){
                allPassFilters[i].prepareToPlay(sampleRate, roomSize);
            }
        }

        void setValue(float sampleRate, float roomSize){
            for(int i = 0; i < 4; i++){
                allPassFilters[i].setValue(sampleRate, roomSize);
            }
        }//这个在roomSize改变时调用，更新每个全通滤波器的

        float processSample(float inputSample, float diffusion){
            //四个全通滤波器串联处理输入信号，输出为最后一个全通滤波器的输出
            float outputSample = inputSample;
            for(int i = 0; i < 4; i++){
                outputSample = allPassFilters[i].processSample(outputSample, diffusion);
                allPassFilters[i].writeIndex = getCircularBufferIndex(allPassFilters[i].writeIndex + 1, allPassBufferSize);
            }

            return outputSample;
        }
    };

    std::vector<allPassFilterAll> allPassFilterAlls{2};//左右声道各一个

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDecayLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDiffusionLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDryLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedWetLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDampLevel { 1.0f };
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