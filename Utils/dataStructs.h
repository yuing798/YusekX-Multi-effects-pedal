#pragma once
//这里定义所有的数据结构体
#include <vector>

class FIFO{
    //这个类并非严格意义上的FIFO，只是针对音频实时处理延迟线的需求设计的一个循环缓冲区
private:
    int size;

    int count;//当前缓冲区内的元素个数

    int write;//指向下一个要被写入的元素
    
public:
    int read{0};//因为在流式音频DSP中，读指针和写指针应该都是距离当前处理样本最近的位置
    std::vector<float> buffer;
    FIFO(int size) : size(size), count(0), write(0), buffer(size, 0.0f) {}
    void push(float value){
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
    void pushBuffer(std::vector<float>& inputBuffer){
        for(float value : inputBuffer){
            push(value);
        }
    }
};