#pragma once
//这里定义所有的数据结构体
#include <vector>

class FIFO{
private:
    int size;

    int count;//当前缓冲区内的元素个数
    int head;//指向下一个要被读取的元素
    int tail;//指向下一个要被写入的元素
public:
    std::vector<float> buffer;
    FIFO(int size) : size(size), count(0), head(0), tail(0), buffer(size, 0.0f) {}
    void push(float value){
        if(count < size){
            buffer[tail] = value;
            tail = (tail + 1) % size;
            count++;
        }else{
            //缓冲区已满，覆盖最旧的数据
            buffer[tail] = value;
            tail = (tail + 1) % size;
            head = (head + 1) % size; //移动头指针，丢弃最旧的数据
        }
    }

    //推入一个数组
    void pushBuffer(std::vector<float>& buffer){
        for(float value : buffer){
            push(value);
        }
    }
};