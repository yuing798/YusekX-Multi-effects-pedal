#include "table.h"
#include <vector>
#include <cmath>
#include "constants.h"

void SineLookUpTable(std::vector<float>& sineTable, int tableSize){
   
    sineTable.resize(tableSize);

    for(int index = 0; index < tableSize; index++){

        float phase = (static_cast<float>(index) / tableSize) * two_pi;
        sineTable[index]= std::sin(phase);        
    }
}

float tanhLookUp(float x){
    //使用双曲正切函数的近似公式
    if (x < -3.0f) return -1.0f;
    if (x > 3.0f) return 1.0f;
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}