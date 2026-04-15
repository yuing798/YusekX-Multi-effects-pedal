#pragma once
#include "Utils/constants.h"
//此处放置所有模块都需要用到的数学函数定义

//线性插值函数
float getLinearInterpolator(const float* data,int size, float index) ;

//环形缓冲区避免索引越界函数
template <typename T1, typename T2>
T1 getCircularBufferIndex(T1 currentIndex, T2 size){
    if(currentIndex >= size) {
        return currentIndex - size;
    }
    else if(currentIndex < 0) {
        return currentIndex + size;
    }
    else {
        return currentIndex;
    }
}
//将弧度转化为索引步长
float transformRadIntoIndexStep(float rad, int tableSize);

//将弧度变成毫秒数
float transformRadIntoMs(float rad, float sampleRate);

//将毫秒转换为样本数
float transformMsIntoSamples(float ms, float sampleRate);