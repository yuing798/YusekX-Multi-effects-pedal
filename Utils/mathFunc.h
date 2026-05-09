#pragma once
#include "Utils/constants.h"
//此处放置所有模块都需要用到的数学函数定义

//线性插值函数
float getLinearInterpolator(const float* data,int size, float index) ;

//三阶拉格朗日插值
float getLagrangeInterpolator(const float* data, int size, float index);

//卷积计算公式
std::vector<float> convolve(const std::vector<float>& input, const std::vector<float>& kernel);

//环形缓冲区避免索引越界函数
template <typename T1>
T1 getCircularBufferIndex(T1 currentIndex, int size){
    if(currentIndex >= size) {
        
        while(currentIndex >= size){
            currentIndex -= size;
        }
        return currentIndex;
    }
    else if(currentIndex < 0) {
        while(currentIndex < 0){
            currentIndex += size;
        }
        return currentIndex;
    }
    return currentIndex;
}
//将弧度转化为索引步长
float rad2IndexStep(float rad, int tableSize);
//将弧度变成毫秒数
float rad2Ms(float rad, float sampleRate);
//将毫秒转换为样本数
float ms2Samples(float ms, float sampleRate);
//辗转相除法求最大公约数
int gcd(int a, int b);
//查找最近素数
int getNearestPrimeNumber(float num);
//sinc计算
float sinc(float x);
//线性转化为lg坐标并进行归一化
float linearToLg(float linearValue);
//线性转化为dB坐标
float linearToDb(float linearValue);
//归一化lg坐标转化为线性
float normalLg2Linear(float normalLgValue, float maxLinearValue, float minLinearValue);