#pragma once
#include <cstddef>
#include <vector>
#include "juce_dsp/juce_dsp.h"
#include "mathFunc.h"
#include "dataStructs.h"

class OverSampling{
private:
    
    int numCoefficients;//FIR系数个数
    int oversamplingFactor;//过采样倍数
    int kernelSize;//卷积核大小
    std::vector<float> FIRCoefficients;//FIR滤波器系数
    std::vector<std::vector<float>> multiPhaseFIRCoefficientsMatrix;//多相分解的FIR系数矩阵
    FIFO oversamplingFifo;


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
        }
        juce::dsp::WindowingFunction<float> window{static_cast<size_t>(numCoefficients), juce::dsp::WindowingFunction<float>::hann};
        window.multiplyWithWindowingTable(coefficients.data(), static_cast<size_t>(numCoefficients));
        return coefficients;
    }

    //将FIR系数填充到多相分解矩阵中
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
        oversamplingFifo(numCoefficients)
    {
        if(numCoefficients % oversamplingFactor != 0){
            //确保系数个数能够被过采样倍数整除
            throw std::invalid_argument("numCoefficients must be divisible by oversamplingFactor");
        }
        setKernelSize();
        setMultiPhaseFIRCoefficientsMatrix();

    }

    ~OverSampling() = default;

    //多支路并行相加
    std::vector<float> processUpSampling(const std::vector<float>& inputBuffer){

    }

    //将inputBuffer长度使用填充0方法变成原来的oversamplingFactor倍长
    std::vector<float> setUpSamplingInputBuffer(const std::vector<float>& inputBuffer){
        std::vector<float> upSampledInputBuffer(inputBuffer.size() * oversamplingFactor, 0.0f);
        for(size_t i = 0; i < inputBuffer.size(); i++){
            upSampledInputBuffer[i * oversamplingFactor] = inputBuffer[i];
        }
        return upSampledInputBuffer;
    }

    //直接进行的processUpSampling方法，作为对比
    float processUpSamplingDirect(float value){
        std::vector<float> buffer;
        buffer.resize(oversamplingFactor, 0.0f);
        buffer[0] = value;
        oversamplingFifo.pushBuffer(buffer);
        float outputValue = 0.0f;
        for(int index = 0; index < numCoefficients; index++){
            outputValue += oversamplingFifo.buffer[index] * FIRCoefficients[index];
        }
        return outputValue;
    }

};