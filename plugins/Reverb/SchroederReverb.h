#pragma once

#include <JuceHeader.h>
#include <vector>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
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
    juce::Slider mixLevelSlider;//干湿混合度
    juce::Slider dampLevelSlider;//低通滤波器的截止频率
    juce::Slider roomSizeSlider;//房间尺寸，影响延迟时间
    juce::Slider baseDelayTimeMsSlider;//基础延迟时间，影响初始反射的时间
    juce::Slider makeUpGainSlider;//补偿增益

	juce::Label decayLevelLabel;
    juce::Label diffusionLevelLabel;
    juce::Label mixLevelLabel;
    juce::Label dampLevelLabel;
    juce::Label roomSizeLabel;
    juce::Label baseDelayTimeMsLabel;
    juce::Label makeUpGainLabel;

	std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> decayLevelAttachment;
    std::unique_ptr<SliderAttachment> diffusionLevelAttachment;
    std::unique_ptr<SliderAttachment> mixLevelAttachment;
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
    float mixLevel { 0.5f };//干湿混合度
    float dampLevel { 0.5f };//低通滤波器系数
    float roomSize { 0.5f };//房间尺寸，影响延迟时间
    float baseDelayTimeMs { 50.0f };//基础延迟时间，单位为毫秒
    float makeUpGainDB { 0.0f };//补偿增益
    float makeUpGain{0.0f};//补偿增益线性值

    std::vector<float> dryTable;//干信号增益查找表，避免每次处理都进行powf计算
    std::vector<float> wetTable;//湿信号增益查找表，避免每次处理都进行powf计算

    preDelay preDelayL;
    preDelay preDelayR;

    struct combFilterSingle{//单个反馈梳状滤波器结构体
        //y[n] = x[n] + decay * y[n - delaySamples]
        std::vector<float> combDelayLineBuffer;
    
        int writeIndex{0};//写指针八个滤波器共用同一个
        float delaySamplesNum{0.0f};
        int combDelayLineValue{0};//根据采样率和房间尺寸计算出的基础延迟时间对应的样本数，作为buffer大小的参考值

        struct lowPassFilter{
            float b0{1.0f};
            float a1{0.0f};

            float y1{0.0f};//上一个输出样本

            void prepareToPlay(float dampLevel){
                a1 = -dampLevel;
                b0 = 1.0f - dampLevel;
            }

            void setValue(float dampLevel){
                prepareToPlay(dampLevel);
            }

            float processSample(float inputSample){
                float outputSample = b0 * inputSample - a1 * y1;
                y1 = outputSample;
                return outputSample;
            }
        }dampFilter;//每个梳状滤波器内置一个低通滤波器，用于模拟高频衰减

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
        combFilterSingle comb1;
        combFilterSingle comb2;
        combFilterSingle comb3;
        combFilterSingle comb4;
        combFilterSingle comb5;
        combFilterSingle comb6;
        combFilterSingle comb7;
        combFilterSingle comb8;

        int combBufferSize{0};

        void setCombBufferSize(float sampleRate){

            combBufferSize = static_cast<int>(combDelayLineLookUp[7] * sampleRate / defaultSampleRate * 2.1) + 1;
            //乘以2是为了在房间尺寸为2时也能保证足够的延迟时间，避免出现死锁问题
            //所有梳状滤波器共用同一个缓冲区，大小根据最长的延迟时间来设置，确保在任何房间尺寸下都能正常工作
            comb1.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            comb2.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            comb3.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            comb4.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            comb5.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            comb6.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            comb7.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            comb8.combDelayLineBuffer.resize(combBufferSize, 0.0f);
        }

        void setCombDelayLineValue(){
            comb1.combDelayLineValue = combDelayLineLookUp[0];
            comb2.combDelayLineValue = combDelayLineLookUp[1];
            comb3.combDelayLineValue = combDelayLineLookUp[2];
            comb4.combDelayLineValue = combDelayLineLookUp[3];
            comb5.combDelayLineValue = combDelayLineLookUp[4];
            comb6.combDelayLineValue = combDelayLineLookUp[5];
            comb7.combDelayLineValue = combDelayLineLookUp[6];
            comb8.combDelayLineValue = combDelayLineLookUp[7];
        }

        void prepareToPlay(float sampleRate, float roomSize, float dampHz){
            setCombDelayLineValue();
            setCombBufferSize(sampleRate);
            comb1.prepareToPlay(sampleRate, roomSize, dampHz);
            comb2.prepareToPlay(sampleRate, roomSize, dampHz);
            comb3.prepareToPlay(sampleRate, roomSize, dampHz);
            comb4.prepareToPlay(sampleRate, roomSize, dampHz);
            comb5.prepareToPlay(sampleRate, roomSize, dampHz);
            comb6.prepareToPlay(sampleRate, roomSize, dampHz);
            comb7.prepareToPlay(sampleRate, roomSize, dampHz);
            comb8.prepareToPlay(sampleRate, roomSize, dampHz);
        }

        void setValue(float sampleRate, float roomSize, float dampLevel){
            comb1.setValue(sampleRate, roomSize, dampLevel);
            comb2.setValue(sampleRate, roomSize, dampLevel);
            comb3.setValue(sampleRate, roomSize, dampLevel);
            comb4.setValue(sampleRate, roomSize, dampLevel);
            comb5.setValue(sampleRate, roomSize, dampLevel);
            comb6.setValue(sampleRate, roomSize, dampLevel);
            comb7.setValue(sampleRate, roomSize, dampLevel);
            comb8.setValue(sampleRate, roomSize, dampLevel);
        }//这个在roomSize改变时调用，更新每个梳状滤波器的delaySamplesNum值

        float processSample(float inputSample, float decay){
            //八个梳状滤波器并行处理输入信号，输出相加
            float outputSample = 0.0f;
            outputSample += comb1.processSample(inputSample, decay);
            outputSample += comb2.processSample(inputSample, decay);
            outputSample += comb3.processSample(inputSample, decay);
            outputSample += comb4.processSample(inputSample, decay);
            outputSample += comb5.processSample(inputSample, decay);
            outputSample += comb6.processSample(inputSample, decay);
            outputSample += comb7.processSample(inputSample, decay);
            outputSample += comb8.processSample(inputSample, decay);

            comb1.writeIndex = getCircularBufferIndex(comb1.writeIndex + 1, combBufferSize);
            comb2.writeIndex = getCircularBufferIndex(comb2.writeIndex + 1, combBufferSize);
            comb3.writeIndex = getCircularBufferIndex(comb3.writeIndex + 1, combBufferSize);
            comb4.writeIndex = getCircularBufferIndex(comb4.writeIndex + 1, combBufferSize);
            comb5.writeIndex = getCircularBufferIndex(comb5.writeIndex + 1, combBufferSize);
            comb6.writeIndex = getCircularBufferIndex(comb6.writeIndex + 1, combBufferSize);
            comb7.writeIndex = getCircularBufferIndex(comb7.writeIndex + 1, combBufferSize);
            comb8.writeIndex = getCircularBufferIndex(comb8.writeIndex + 1, combBufferSize);

            return outputSample / 8.0f;//平均输出，避免增益过大
        }
    };

    combFilterAll combFiltersL;
    combFilterAll combFiltersR;



    struct allPassFilterSingle{//单个全通滤波器结构体
        //y[n] = diffusion * x[n] + x[n - delaySamples] - diffusion * y[n - delaySamples]
        std::vector<float> allPassDelayLineBuffer;

        int writeIndex{0};
        int delaySamplesNum{0};
        int allPassDelayLineValue{0};//根据采样率和房间尺寸计算出的基础延迟时间对应的样本数，作为buffer大小的参考值

        void prepareToPlay(float sampleRate, float roomSize){
            delaySamplesNum = getNearestPrimeNumber(allPassDelayLineValue * sampleRate / defaultSampleRate * roomSize);
            writeIndex = 0;
        }

        void setValue(float sampleRate, float roomSize){
            delaySamplesNum = getNearestPrimeNumber(allPassDelayLineValue * sampleRate / defaultSampleRate * roomSize);
        }

        float processSample(float inputSample, float diffusion){
            int readIndex = getCircularBufferIndex(writeIndex - delaySamplesNum, static_cast<int>(allPassDelayLineBuffer.size()));
            float readSample = allPassDelayLineBuffer[readIndex];
            float outputSample = - diffusion * inputSample + readSample ;
            allPassDelayLineBuffer[writeIndex] = inputSample + diffusion * readSample;
            return outputSample;
        }
    };

    struct allPassFilterAll{
        allPassFilterSingle allPass1;
        allPassFilterSingle allPass2;
        allPassFilterSingle allPass3;
        allPassFilterSingle allPass4;

        int allPassBufferSize{0};
        int writeIndex{0};//四个滤波器共用一个写指针，确保它们的写位置始终保持一致

        void setAllPassBufferSize(float sampleRate){
            allPassBufferSize = static_cast<int>(allPassDelayLineLookUp[3] * sampleRate / defaultSampleRate * 2.1) + 1;
            //乘以2是为了在房间尺寸为2时也能保证足够的延迟时间，避免出现死锁问题
            //所有全通滤波器共用同一个缓冲区，大小根据最长的延迟时间来设置，确保在任何房间尺寸下都能正常工作
            allPass1.allPassDelayLineBuffer.resize(allPassBufferSize, 0.0f);
            allPass2.allPassDelayLineBuffer.resize(allPassBufferSize, 0.0f);
            allPass3.allPassDelayLineBuffer.resize(allPassBufferSize, 0.0f);
            allPass4.allPassDelayLineBuffer.resize(allPassBufferSize, 0.0f);
        }

        void setAllPassDelayLineValue(){
            allPass1.allPassDelayLineValue = allPassDelayLineLookUp[0];
            allPass2.allPassDelayLineValue = allPassDelayLineLookUp[1];
            allPass3.allPassDelayLineValue = allPassDelayLineLookUp[2];
            allPass4.allPassDelayLineValue = allPassDelayLineLookUp[3];
        }

        void prepareToPlay(float sampleRate, float roomSize){
            setAllPassDelayLineValue();
            setAllPassBufferSize(sampleRate);
            allPass1.prepareToPlay(sampleRate, roomSize);
            allPass2.prepareToPlay(sampleRate, roomSize);
            allPass3.prepareToPlay(sampleRate, roomSize);
            allPass4.prepareToPlay(sampleRate, roomSize);
        }

        void setValue(float sampleRate, float roomSize){
            allPass1.setValue(sampleRate, roomSize);
            allPass2.setValue(sampleRate, roomSize);
            allPass3.setValue(sampleRate, roomSize);
            allPass4.setValue(sampleRate, roomSize);
        }//这个在roomSize改变时调用，更新每个全通滤波器的

        float processSample(float inputSample, float diffusion){
            //四个全通滤波器串联处理输入信号，输出为最后一个全通滤波器的输出
            float outputSample = inputSample;
            outputSample = allPass1.processSample(outputSample, diffusion);
            outputSample = allPass2.processSample(outputSample, diffusion);
            outputSample = allPass3.processSample(outputSample, diffusion);
            outputSample = allPass4.processSample(outputSample, diffusion);

            allPass1.writeIndex = getCircularBufferIndex(allPass1.writeIndex + 1, allPassBufferSize);
            allPass2.writeIndex = getCircularBufferIndex(allPass2.writeIndex + 1, allPassBufferSize);
            allPass3.writeIndex = getCircularBufferIndex(allPass3.writeIndex + 1, allPassBufferSize);
            allPass4.writeIndex = getCircularBufferIndex(allPass4.writeIndex + 1, allPassBufferSize);

            return outputSample;
        }
    };

    allPassFilterAll allPassFiltersL;
    allPassFilterAll allPassFiltersR;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDecayLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDiffusionLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMixLevel { 1.0f };
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