#include "mathFunc.h"
#include <cmath>

float getLinearInterpolator(const float* data,int size, float index){
    int index1 = static_cast<int>(index);
    int index2 = getCircularBufferIndex(index1 + 1, size); 
    float fraction = index - index1;

    return (1.0f - fraction) * data[index1] + fraction * data[index2];
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
    for(int i = value; i > 2; i--){
        bool isPrime = true;
        for(int j = 2; j <= std::sqrt(i); j++){
            if(i % j == 0){
                isPrime = false;
                break;
            }
        }
        if(isPrime) return i;
    }//左边和右边都查找一遍
}