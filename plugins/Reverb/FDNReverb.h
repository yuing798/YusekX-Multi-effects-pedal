#pragma once

#include <JuceHeader.h>
#include <cstddef>
#include <vector>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "Utils/mathFunc.h"
#include "dspFilters.h"

class FDNReverbEditor final : public juce::Component
{
private:

	juce::Label mTitle;
	juce::TextButton mOpenCloseButton { "Open" };

	juce::Slider decayLevelSlider;//反馈梳状滤波器反馈度
    juce::Slider dryLevelSlider;
    juce::Slider wetLevelSlider;
    juce::Slider dampLevelSlider;//低通滤波器的截止频率
    juce::Slider roomSizeSlider;//房间尺寸，影响延迟时间
    juce::Slider preDelayTimeMsSlider;//基础延迟时间，影响初始反射的时间
    juce::Slider makeUpGainSlider;//补偿增益

	juce::Label decayLevelLabel;
    juce::Label dryLevelLabel;
    juce::Label wetLevelLabel;
    juce::Label dampLevelLabel;
    juce::Label roomSizeLabel;
    juce::Label preDelayTimeMsLabel;
    juce::Label makeUpGainLabel;

	std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> decayLevelAttachment;
    std::unique_ptr<SliderAttachment> dryLevelAttachment;
    std::unique_ptr<SliderAttachment> wetLevelAttachment;
    std::unique_ptr<SliderAttachment> dampLevelAttachment;
    std::unique_ptr<SliderAttachment> roomSizeAttachment;
    std::unique_ptr<SliderAttachment> preDelayTimeMsAttachment;
    std::unique_ptr<SliderAttachment> makeUpGainAttachment;

	void bindParameters();

    juce::AudioProcessorValueTreeState& mAPVTS;

public:
	explicit FDNReverbEditor(juce::AudioProcessorValueTreeState& apvts);
	~FDNReverbEditor() override = default;
	void resized() override;

};

class FDNReverbProcessor{
private:
    float currentSampleRate { defaultSampleRate };
    int currentMaximumBlockSize{bufferSize};


    bool isOpen { false };
    float decayLevel { 0.5f };//反馈梳状滤波器反馈度
    float dryLevel { 0.5f };//干信号增益
    float wetLevel { 0.5f };//湿信号增益
    float dampLevel { 0.5f };//低通滤波器系数
    float roomSize { 0.5f };//房间尺寸，影响延迟时间
    float preDelayTimeMs { 50.0f };//预延迟时间，单位为毫秒
    float makeUpGainDB { 0.0f };//补偿增益
    float makeUpGain{0.0f};//补偿增益线性值

    preDelay preDelayL;
    preDelay preDelayR;

    struct FDNDelayLine{//单个反馈梳状滤波器结构体
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
            //combDelayLineBuffer[writeIndex] += inputSample + decay * readSample;
            //这时候还不要把输入信号和反馈信号叠加写入缓冲区，因为在FDN网络中还要进行一次写入缓冲区
            //如果这时候就把叠加信号写进缓冲区的话
            //缓冲区会出现两倍的叠加信号
            //writeIndex = getCircularBufferIndex(writeIndex + 1, combDelayLineBuffer.size());
            return inputSample + decay * readSample;
        }
    };

    struct FDNNetwork{
        
        int numLines { 8 };//八个并行的反馈梳状滤波器
        std::vector<FDNDelayLine> FDNDelayLines { static_cast<size_t>(numLines) };//FDN延迟线数组
        std::vector<float> outputLines;//第一次从延迟线出来的数据
        std::vector<float> inputLines;//第一次从延迟线出来的数据经过反馈网络后的输入
        int combBufferSize{0};

        void setCombBufferSize(float sampleRate){

            combBufferSize = static_cast<int>(combDelayLineLookUp[7] * sampleRate / defaultSampleRate * 2.1) + 1;
            //乘以2.1是为了在房间尺寸为2(最大)时也能保证足够的延迟时间，
            // 因为最近素数查找算法会返回一个比理论值大的素数，乘以2.1可以保证在任何房间尺寸下都能满足最长延迟时间的需求
            //所有梳状滤波器共用同一个缓冲区，大小根据最长的延迟时间来设置，确保在任何房间尺寸下都能正常工作
            for(auto& delayLine : FDNDelayLines){
                delayLine.combDelayLineBuffer.resize(combBufferSize, 0.0f);
            }
        }

        void setCombDelayLineValue(){
            for(int i = 0; i < numLines; ++i){
                FDNDelayLines[i].combDelayLineValue = combDelayLineLookUp[i];
            }
        }

        void prepareToPlay(float sampleRate, float roomSize, float dampHz){
            setCombDelayLineValue();
            setCombBufferSize(sampleRate);
            for(int i = 0; i < numLines; ++i){
                FDNDelayLines[i].prepareToPlay(sampleRate, roomSize, dampHz);
            }
            outputLines.resize(numLines, 0.0f);
            inputLines.resize(numLines, 0.0f);
        }

        void setValue(float sampleRate, float roomSize, float dampLevel){
            for(int i = 0; i < numLines; ++i){
                FDNDelayLines[i].setValue(sampleRate, roomSize, dampLevel);
            }
        }//这个在roomSize改变时调用，更新每个梳状滤波器的delaySamplesNum值

        float processSample(float inputSample, float decay){
            std::fill(outputLines.begin(), outputLines.end(), 0.0f);
            std::fill(inputLines.begin(), inputLines.end(), 0.0f);
            for(int i = 0; i < numLines; ++i){
                outputLines[i] = FDNDelayLines[i].processSample(inputSample, decay);
            }

            for(int i = 0; i < numLines; ++i){
                for(int j = 0; j < numLines; ++j){
                    inputLines[i] += hadamard8x8[i][j] * outputLines[j];
                }
            }

            for(int i = 0; i < numLines; ++i){
                FDNDelayLines[i].combDelayLineBuffer[FDNDelayLines[i].writeIndex] = inputLines[i];
                //将反馈网络的输出叠加到每个梳状滤波器的输入中
                FDNDelayLines[i].writeIndex = getCircularBufferIndex(FDNDelayLines[i].writeIndex + 1, FDNDelayLines[i].combDelayLineBuffer.size());
            }

            float outputSample = 0.0f;
            for(int i = 0; i < numLines; ++i){
                outputSample += outputLines[i];
            }
            return outputSample / numLines;//对输出进行平均，防止增益过大
        }
    };

    FDNNetwork FDNNetworkL;
    FDNNetwork FDNNetworkR;


    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDecayLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDryLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedWetLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDampLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedRoomSize { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedPreDelayTimeMs { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMakeUpGainDB { 1.0f };

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    juce::AudioProcessorValueTreeState& mAPVTS;

    

public:
    explicit FDNReverbProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~FDNReverbProcessor() = default;
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