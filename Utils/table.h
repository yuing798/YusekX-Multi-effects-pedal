#pragma once
//这里定义所有的查表类函数

#include <vector>
void RadSineLookUpTable(std::vector<float>& sineTable, int tableSize);
void AngleSineLookUpTable(std::vector<float>& sineTable, int tableSize);

float tanhApproximate(float x);

//创建cos余弦表
void CosLookUpTable(std::vector<float>& cosTable, int tableSize);