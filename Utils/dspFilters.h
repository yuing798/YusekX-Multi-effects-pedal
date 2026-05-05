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

    void prepareToPlay(float maxDelayTimeMs, float sampleRate){
        float maxDelaySamplesNum = transformMsIntoSamples(maxDelayTimeMs , sampleRate);//200.0f是最大预延迟时间
        preDelayBuffer.resize(maxDelaySamplesNum + 1, 0.0f);
        writeIndex = 0;
    }

    void setValue(float sampleRate, float baseDelayTimeMs){
        delaySamplesNum = transformMsIntoSamples(baseDelayTimeMs, sampleRate);
    }

    float processSample(float inputSample){//这个是普通的延迟效果器y[n] = x[n-D]函数
        float readIndex = getCircularBufferIndex(writeIndex - delaySamplesNum, preDelayBuffer.size());
        float readSample = getLinearInterpolator(preDelayBuffer.data(), static_cast<int>(preDelayBuffer.size()), readIndex);
        writeIndex = getCircularBufferIndex(writeIndex + 1, preDelayBuffer.size());
        preDelayBuffer[writeIndex] = inputSample;
        return readSample;
    }

    float processSample(float inputSample, float delaySamplesNums){//这个在确切知道群时延样本数时使用
        float readIndex = getCircularBufferIndex(writeIndex - delaySamplesNums, preDelayBuffer.size());
        float readSample = getLinearInterpolator(preDelayBuffer.data(), static_cast<int>(preDelayBuffer.size()), readIndex);
        writeIndex = getCircularBufferIndex(writeIndex + 1, preDelayBuffer.size());
        preDelayBuffer[writeIndex] = inputSample;
        return readSample;
    }
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


    void setKernelSize(){
        kernelSize = numCoefficients / oversamplingFactor;
    }

    //计算FIR系数，使用sinc函数生成理想低通滤波器的冲激响应，并应用汉宁窗进行加窗处理
    std::vector<float> setFIRCoefficients(){
        int FIROrders = numCoefficients - 1;
        //滤波器的阶数定义为滤波器差分方程中最大延迟的样本数
        std::vector<float> coefficients;
        coefficients.resize(numCoefficients, 0.0f);
        //阶数等于系数个数-1

        for(int index = 0; index < numCoefficients;index++){
            coefficients[index] = sinc(static_cast<float>(index - static_cast<float>(FIROrders) / 2));
        }//sinc函数的傅里叶变化为矩形窗，是理想的低通滤波器的冲激响应
        //这里让sinc函数进行时移，目的是保证因果性
        juce::dsp::WindowingFunction<float> window{static_cast<size_t>(numCoefficients), juce::dsp::WindowingFunction<float>::hann};
        window.multiplyWithWindowingTable(coefficients.data(), static_cast<size_t>(numCoefficients));
        //加窗是为了抑制sinc函数的旁瓣，减少频谱泄漏对过采样性能的影响

        return coefficients;
    }

    //将FIR系数填充到多相分解矩阵中，用于多相分解的矩阵
    void setMultiPhaseFIRCoefficientsMatrix(){
        size_t rows = oversamplingFactor;
        size_t columns = static_cast<size_t>(kernelSize);
        multiPhaseFIRCoefficientsMatrix.resize(
            oversamplingFactor, 
            std::vector<float>(columns, 0.0f));
        for(size_t phase = 0; phase < rows; phase++){
            for(size_t column = 0; column < columns; column++){
                multiPhaseFIRCoefficientsMatrix[phase][column] = FIRCoefficients[column * rows + phase];
                //矩阵形状
                // 0  4  8  ......
                // 1  5  9  ......
                // 2  6  10 ......
                // 3  7  11 ......
            }
        }
    }

public:
    OverSampling(int numCoefficients, int overSamplingFactor)
        :
        numCoefficients(numCoefficients),
        oversamplingFactor(overSamplingFactor),
        FIRCoefficients(setFIRCoefficients()),
        upSamplingFifoMultiPhase(static_cast<int>(numCoefficients / oversamplingFactor)),
        downSamplingFifoMultiPhase(static_cast<int>(numCoefficients / oversamplingFactor))
        // upSamplingFifoDirect(numCoefficients),
        // downSamplingFifoDirect(numCoefficients)
    {
        if(numCoefficients % oversamplingFactor != 0){
            //确保系数个数能够被过采样倍数整除
            throw std::invalid_argument("numCoefficients must be divisible by oversamplingFactor");
        }
        setKernelSize();
        setMultiPhaseFIRCoefficientsMatrix();
        // upSamplingbuffer.resize(oversamplingFactor, 0.0f);
        upSamplingBufferMultiPhase.resize(oversamplingFactor, 0.0f);
        //内存分配绝对不能在单样本循环中进行，因为内存分配的开销会导致性能灾难

    }

    ~OverSampling() = default;

    //多支路并行相加(多相处理)
    std::vector<float> upSamplingBufferMultiPhase;

    std::vector<float> processUpSamplingMultiPhase(float value){
        //每次处理一个输入样本，输出一个包含过采样倍数个样本的数组

        std::fill(upSamplingBufferMultiPhase.begin(), upSamplingBufferMultiPhase.end(), 0.0f);
        upSamplingFifoMultiPhase.push(value);
        for(int phase = 0; phase < oversamplingFactor; phase++){
            for(int index = 0; index < kernelSize; index++){
                upSamplingBufferMultiPhase[phase] += upSamplingFifoMultiPhase.buffer[
                    getCircularBufferIndex(
                    upSamplingFifoMultiPhase.read - index,
                    //将历史缓冲区分别和四个相位的系数做差分方程
                    
                    upSamplingFifoMultiPhase.buffer.size())] * multiPhaseFIRCoefficientsMatrix[phase][index];
            }
        }
        return upSamplingBufferMultiPhase;
    }

    float processDownSamplingMultiPhase(std::vector<float>& inputBuffer){
        downSamplingFifoMultiPhase.pushBuffer(inputBuffer);
        float outputSample = 0.0f;
        for(int phase = 0; phase <oversamplingFactor; phase++){
            float phaseOutput = 0.0f;
            for(int index = 0; index < kernelSize; index++){
                phaseOutput += downSamplingFifoMultiPhase.buffer[
                    getCircularBufferIndex(
                    downSamplingFifoMultiPhase.read - index * oversamplingFactor - (oversamplingFactor - 1 - phase), 
                    //read当前读取的位置一定是相位3,因为pushBuffer会将四个相位的数据依次推入缓冲区
                    // 所以read指针每次移动4个位置，确保每个相位的数据都能被正确处理
                    //- index * oversamplingFactor是为了实现抽取操作，即每隔oversamplingFactor个样本取一个样本进行处理
                    //- (oversamplingFactor - 1 - phase)是为了对齐，如相位等于0,读取三个样本前的数据；
                    // 相位等于1,读取两个样本前的数据；
                    // 相位等于2,读取一个样本前的数据；
                    // 相位等于3,读取当前的数据

                    downSamplingFifoMultiPhase.buffer.size())] * multiPhaseFIRCoefficientsMatrix[phase][index];
            }
            outputSample += phaseOutput;
        }
        return outputSample;
    }

    //延迟函数，抵消两个FIR滤波器的群时延，确保输入输出样本对齐


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

