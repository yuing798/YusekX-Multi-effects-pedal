#pragma once

//此处放置所有模块都需要用到的数学函数定义

//线性插值函数
float getLinearInterpolator(const float* data,int size, float index) ;

//环形缓冲区避免下一个数越界函数
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