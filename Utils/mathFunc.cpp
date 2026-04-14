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