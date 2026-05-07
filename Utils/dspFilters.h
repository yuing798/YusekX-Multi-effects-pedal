#pragma once
#include <cstddef>
#include <vector>
#include "juce_dsp/juce_dsp.h"
#include "mathFunc.h"
#include "dataStructs.h"

struct preDelay{
    std::vector<float> preDelayBuffer;
    int writeIndex{0};
    float delaySamplesNum{0};

    void prepareToPlay(float maxDelayTimeMs, float sampleRate);

    void setValue(float sampleRate, float baseDelayTimeMs);

    float processSample(float inputSample);

    float processSample(float inputSample, float delaySamplesNums);
};

class OverSampling{
private:
    
    int numCoefficients;//FIR系数个数
    int oversamplingFactor;//过采样倍数
    int kernelSize{0};//卷积核大小
    std::vector<float> FIRCoefficients;//FIR滤波器系数
    std::vector<std::vector<float>> multiPhaseFIRCoefficientsMatrix;//多相分解的FIR系数矩阵
    // FIFO upSamplingFifoDirect;//未使用多相分解的上采样
    // FIFO downSamplingFifoDirect;
    FIFO upSamplingFifoMultiPhase;//使用了多相分解的上采样
    FIFO downSamplingFifoMultiPhase;


    void setKernelSize();

    //计算FIR系数，使用sinc函数生成理想低通滤波器的冲激响应，并应用汉宁窗进行加窗处理
    std::vector<float> setFIRCoefficients();

    //将FIR系数填充到多相分解矩阵中，用于多相分解的矩阵
    void setMultiPhaseFIRCoefficientsMatrix();

public:
    OverSampling(int numCoefficients, int overSamplingFactor);

    ~OverSampling() = default;

    //多支路并行相加(多相处理)
    std::vector<float> upSamplingBufferMultiPhase;

    std::vector<float> processUpSamplingMultiPhase(float value);

    float processDownSamplingMultiPhase(std::vector<float>& inputBuffer);


    // std::vector<float> upSamplingbuffer;

    // //直接进行的processUpSampling方法，作为对比
    // std::vector<float> processUpSamplingDirect(float value){
        
    //     // upSamplingbuffer.assign(oversamplingFactor, 0.0f);
    //     std::fill(upSamplingbuffer.begin(), upSamplingbuffer.end(), 0.0f);
    //     upSamplingbuffer[0] = value;
    //     // upSamplingFifoDirect.pushBuffer(upSamplingbuffer);
    //     for(int i = 0; i < oversamplingFactor; i++){
    //         upSamplingFifoDirect.push(upSamplingbuffer[i]);
    //         upSamplingbuffer[i] = 0.0f;
    //         for(int j = 0; j < numCoefficients; j++){
    //             upSamplingbuffer[i] += upSamplingFifoDirect.buffer[
    //                 getCircularBufferIndex(
    //                 upSamplingFifoDirect.read - j, 
    //                 upSamplingFifoDirect.buffer.size())] * FIRCoefficients[j];
    //         }
    //     }
    //     return upSamplingbuffer;
    // }

    // float processDownSamplingDirect(std::vector<float> inputBuffer){
    //     downSamplingFifoDirect.pushBuffer(inputBuffer);
    //     inputBuffer[0] = 0.0f;

    //     for(int i = 0; i < numCoefficients; i++){
    //         inputBuffer[0] += downSamplingFifoDirect.buffer[
    //             getCircularBufferIndex(
    //             downSamplingFifoDirect.read - i - oversamplingFactor + 1, 
    //             //- oversamplingFactor + 1是为了抑制群时延的影响，确保输出样本与输入样本对齐
    //             downSamplingFifoDirect.buffer.size())] * FIRCoefficients[i];
    //     }
    //     //先进行FIR滤波然后下采样
    //     //因为抽取只取第一个数据，所以后面的数据直接丢弃
    //     return inputBuffer[0];
    // }

};

//一阶低通滤波器
struct lowPassFilter{
    float b0{1.0f};
    float a1{0.0f};

    float y1{0.0f};//上一个输出样本

    void prepareToPlay(float dampLevel);

    void setValue(float dampLevel);

    float processSample(float inputSample);
};

struct allPassFilterSingle{//单个一阶全通滤波器结构体
    //y[n] = diffusion * x[n] + x[n - delaySamples] - diffusion * y[n - delaySamples]
    std::vector<float> allPassDelayLineBuffer;

    int writeIndex{0};
    int delaySamplesNum{0};
    int allPassDelayLineValue{0};//根据采样率和房间尺寸计算出的基础延迟时间对应的样本数，作为buffer大小的参考值

    void prepareToPlay(float sampleRate, float roomSize);

    void setValue(float sampleRate, float roomSize);

    float processSample(float inputSample, float diffusion);
};

