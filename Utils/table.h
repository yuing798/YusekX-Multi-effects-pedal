#pragma once
//这里定义所有的查表类函数

#include <utility>
#include <vector>
void SineLookUpTable(std::vector<float>& sineTable, int tableSize);

float tanhLookUp(float x);

//创建cos余弦表
void CosLookUpTable(std::vector<float>& cosTable, int tableSize);

void dryLookUpTable(std::vector<float>& dryTable, int tableSize);
void wetLookUpTable(std::vector<float>& wetTable, int tableSize);