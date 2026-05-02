#pragma once
#include <cstddef>
#include <vector>
#include "juce_dsp/juce_dsp.h"
#include "mathFunc.h"

class OverSampling{
private:
    
    int numCoefficients;//FIR系数个数
    int oversamplingFactor;//过采样倍数
    int kernelSize;//卷积核大小
    std::vector<float> FIRCoefficients;//FIR滤波器系数
    std::vector<std::vector<float>> multiPhaseFIRCoefficientsMatrix;//多相分解的FIR系数矩阵
    std::vector<float> inputBuffer; //输入缓冲区

public:
    OverSampling(int numCoefficients, int overSamplingFactor)
        :
        numCoefficients(numCoefficients),
        oversamplingFactor(overSamplingFactor),
        FIRCoefficients(setFIRCoefficients(numCoefficients))
    {
        if(numCoefficients % oversamplingFactor != 0){
            //确保系数个数能够被过采样倍数整除
            throw std::invalid_argument("numCoefficients must be divisible by oversamplingFactor");
        }
    }
    void setKernelSize(){
        kernelSize = numCoefficients / oversamplingFactor;
    }

    //计算FIR系数，使用sinc函数生成理想低通滤波器的冲激响应，并应用汉宁窗进行加窗处理
    std::vector<float> setFIRCoefficients(
        size_t numCoefficients//FIR系数个数
    ){
        size_t FIROrders = numCoefficients - 1;
        //滤波器的阶数定义为滤波器差分方程中最大延迟的样本数
        std::vector<float> coefficients;
        coefficients.resize(numCoefficients, 0.0f);
        //阶数等于系数个数-1

        for(size_t index = 0; index < numCoefficients;index++){
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


};