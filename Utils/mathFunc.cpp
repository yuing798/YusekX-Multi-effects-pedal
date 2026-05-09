#include "mathFunc.h"
#include <cmath>

float getLinearInterpolator(const float* data,int size, float index){
    int index1 = static_cast<int>(index);
    int index2 = getCircularBufferIndex(index1 + 1, size); 
    float fraction = index - index1;

    return (1.0f - fraction) * data[index1] + fraction * data[index2];
}

float getLagrangeInterpolator(const float *data, int size, float index){
    // 原理：用一个 N 阶多项式穿过 N+1 个最近的样本点，然后取多项式在所需延迟位置的值。
    // 奇数阶的拉格朗日插值（如 3 阶，用 4 个样本）相当于一个对称的 FIR 滤波器，其系数是分数延迟 d 的简单多项式。
    int intIndex = static_cast<int>(index);
    float distanceIndex2IntIndex = index - intIndex;
    float nagetive1 = data[getCircularBufferIndex(intIndex - 1, size)];
    float intSample = data[intIndex];
    float positive1 = data[getCircularBufferIndex(intIndex + 1, size)];
    float positive2 = data[getCircularBufferIndex(intIndex + 2, size)];

    float outputSample = 0.0f;
    float distanceIndex2IntIndexMinus1 = distanceIndex2IntIndex - 1;
    float distanceIndex2IntIndexMinus2 = distanceIndex2IntIndex - 2;
    float distanceIndex2IntIndexPlus1 = distanceIndex2IntIndex + 1;
    
    outputSample -= distanceIndex2IntIndex * distanceIndex2IntIndexMinus1 * distanceIndex2IntIndexMinus2 * 0.16667f * nagetive1;
    outputSample += distanceIndex2IntIndexPlus1 * distanceIndex2IntIndexMinus1 * distanceIndex2IntIndexMinus2 * 0.5f * intSample;
    outputSample -= distanceIndex2IntIndexPlus1 * distanceIndex2IntIndex * distanceIndex2IntIndexMinus2 * 0.5f * positive1;
    outputSample += distanceIndex2IntIndexPlus1 * distanceIndex2IntIndex * distanceIndex2IntIndexMinus1 * 0.16667f * positive2;
    return outputSample;
}

std::vector<float> convolve(const std::vector<float>& input, const std::vector<float>& kernel){
    int inputSize = static_cast<int>(input.size());
    int kernelSize = static_cast<int>(kernel.size());
    int outputSize = inputSize + kernelSize - 1;

    std::vector<float> output(outputSize, 0.0f);

    for (int i = 0; i < outputSize; ++i) {
        for (int j = 0; j < kernelSize; ++j) {
            if (i - j >= 0 && i - j < inputSize) {
                output[i] += input[i - j] * kernel[j];
            }
        }
    }

    return output;
}

//将弧度转化为正弦索引步长
float rad2IndexStep(float rad, int tableSize)
{
    return (rad / two_pi) * static_cast<float>(tableSize);
}

//将弧度转化为毫秒数
float rad2Ms(float rad, float sampleRate){
    return (two_pi / rad) * (1000.0f / sampleRate);
}


//将毫秒转换为样本数
float ms2Samples(float ms, float sampleRate){
    return (ms / 1000.0f) * sampleRate;
}

int gcd(int a,int b){
    int temp = 1;
    int max = std::max(a, b);
    int min = std::min(a, b);
    while(min != 0){
        temp = min;
        min = max % min;
        max = temp;
    }
    return max;
}

int getNearestPrimeNumber(float num){
    int value = static_cast<int>(num);
    if(num <= 2) return 2;
    for(int i = value; ; i++){
        bool isPrime = true;
        for(int j = 2; j <= std::sqrt(i); j++){
            if(i % j == 0){
                isPrime = false;
                break;
            }
        }
        if(isPrime) return i;
    }
}

float sinc(float x){
    if (x == 0.0f) {
        return 1.0f; // sinc(0) 的极限值为 1
    } else {
        return std::sin(pi * x) / (pi * x);
    }
}

float linearToLg(float linearValue){
    if(linearValue <= 0.0f) return -100.0f; // 避免对数函数的负无穷大
    return std::log10(linearValue);
}
float linearToDb(float linearValue){
    return 20.0f * linearToLg(linearValue);
}
float normalLg2Linear(float normalLgValue, float maxLinearValue, float minLinearValue){
    minLinearValue = std::max(minLinearValue, 1e-6f); // 避免对数函数的负无穷大
    return minLinearValue * std::pow(maxLinearValue / minLinearValue, normalLgValue);
}