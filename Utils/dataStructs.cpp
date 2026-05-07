#include "dataStructs.h"

void FIFO::push(float value){
    if(count < size){
        buffer[write] = value;
        read = write; //更新读指针为当前写指针位置
        write = (write + 1) % size;
        count++;
    }else{
        //缓冲区已满，覆盖最旧的数据
        buffer[write] = value;
        read = write; //更新读指针为当前写指针位置
        write = (write + 1) % size;
    }
}

//推入一个数组
void FIFO::pushBuffer(std::vector<float>& inputBuffer){
    for(float value : inputBuffer){
        push(value);
    }
}