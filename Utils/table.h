#pragma once
//这里定义所有的查表类函数

#include <utility>
#include <vector>
void SineLookUpTable(std::vector<float>& sineTable, int tableSize);

float tanhLookUp(float x);

//使用cos和sin查表法保证非相干信号相加为1，比如wet^2 + dry^2 = 1
std::pair<std::vector<float>, std::vector<float>> createCosSinLookUpTables(int tableSize);