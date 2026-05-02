#include "mathFunc.h"
#include <cmath>

float getLinearInterpolator(const float* data,int size, float index){
    int index1 = static_cast<int>(index);
    int index2 = getCircularBufferIndex(index1 + 1, size); 
    float fraction = index - index1;

    return (1.0f - fraction) * data[index1] + fraction * data[index2];
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
float transformRadIntoIndexStep(float rad, int tableSize)
{
    return (rad / two_pi) * static_cast<float>(tableSize);
}

//将弧度转化为毫秒数
float transformRadIntoMs(float rad, float sampleRate){
    return (two_pi / rad) * (1000.0f / sampleRate);
}


//将毫秒转换为样本数
float transformMsIntoSamples(float ms, float sampleRate){
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