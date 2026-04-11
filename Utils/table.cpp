#include "table.h"
#include <vector>
#include <cmath>
#include "constants.h"

std::vector<float> SineLookUpTable(int tableSize){

    std::vector<float> sineTable;
    
    sineTable.resize(tableSize);

    for(int index = 0; index < tableSize; index++){

        float phase = (static_cast<float>(index) / tableSize) * two_pi;
        sineTable[index]= std::sin(phase);        
    }
    return sineTable;
}