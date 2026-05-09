#include "dspFilters.h"

void preDelay::prepareToPlay(float maxDelayTimeMs, float sampleRate){
    float maxDelaySamplesNum = ms2Samples(maxDelayTimeMs , sampleRate);//200.0f是最大预延迟时间
    preDelayBuffer.resize(maxDelaySamplesNum + 1, 0.0f);
    writeIndex = 0;
}

void preDelay::setValue(float sampleRate, float baseDelayTimeMs){
    delaySamplesNum = ms2Samples(baseDelayTimeMs, sampleRate);
}

float preDelay::processSample(float inputSample){//这个是普通的延迟效果器y[n] = x[n-D]函数
    float readIndex = getCircularBufferIndex(writeIndex - delaySamplesNum, preDelayBuffer.size());
    float readSample = getLinearInterpolator(preDelayBuffer.data(), static_cast<int>(preDelayBuffer.size()), readIndex);
    writeIndex = getCircularBufferIndex(writeIndex + 1, preDelayBuffer.size());
    preDelayBuffer[writeIndex] = inputSample;
    return readSample;
}

float preDelay::processSample(float inputSample, float delaySamplesNums){//这个在确切知道群时延样本数时使用
    float readIndex = getCircularBufferIndex(writeIndex - delaySamplesNums, preDelayBuffer.size());
    float readSample = getLinearInterpolator(preDelayBuffer.data(), static_cast<int>(preDelayBuffer.size()), readIndex);
    writeIndex = getCircularBufferIndex(writeIndex + 1, preDelayBuffer.size());
    preDelayBuffer[writeIndex] = inputSample;
    return readSample;
}

void OverSampling::setKernelSize(){
    kernelSize = numCoefficients / oversamplingFactor;
}

//计算FIR系数，使用sinc函数生成理想低通滤波器的冲激响应，并应用汉宁窗进行加窗处理
std::vector<float> OverSampling::setFIRCoefficients(){
    int FIROrders = numCoefficients - 1;
    //滤波器的阶数定义为滤波器差分方程中最大延迟的样本数
    std::vector<float> coefficients;
    coefficients.resize(numCoefficients, 0.0f);
    //阶数等于系数个数-1

    for(int index = 0; index < numCoefficients;index++){
        coefficients[index] = sinc(static_cast<float>(index - static_cast<float>(FIROrders) / 2));
    }//sinc函数的傅里叶变化为矩形窗，是理想的低通滤波器的冲激响应
    //这里让sinc函数进行时移，目的是保证因果性
    juce::dsp::WindowingFunction<float> window{static_cast<size_t>(numCoefficients), juce::dsp::WindowingFunction<float>::hann};
    window.multiplyWithWindowingTable(coefficients.data(), static_cast<size_t>(numCoefficients));
    //加窗是为了抑制sinc函数的旁瓣，减少频谱泄漏对过采样性能的影响

    return coefficients;
}

//将FIR系数填充到多相分解矩阵中，用于多相分解的矩阵
void OverSampling::setMultiPhaseFIRCoefficientsMatrix(){
    size_t rows = oversamplingFactor;
    size_t columns = static_cast<size_t>(kernelSize);
    multiPhaseFIRCoefficientsMatrix.resize(
        oversamplingFactor, 
        std::vector<float>(columns, 0.0f));
    for(size_t phase = 0; phase < rows; phase++){
        for(size_t column = 0; column < columns; column++){
            multiPhaseFIRCoefficientsMatrix[phase][column] = FIRCoefficients[column * rows + phase];
            //矩阵形状
            // 0  4  8  ......
            // 1  5  9  ......
            // 2  6  10 ......
            // 3  7  11 ......
        }
    }
}


OverSampling::OverSampling(int numCoefficients, int overSamplingFactor)
    :
    numCoefficients(numCoefficients),
    oversamplingFactor(overSamplingFactor),
    FIRCoefficients(setFIRCoefficients()),
    upSamplingFifoMultiPhase(static_cast<int>(numCoefficients / oversamplingFactor)),
    downSamplingFifoMultiPhase(static_cast<int>(numCoefficients / oversamplingFactor))
    // upSamplingFifoDirect(numCoefficients),
    // downSamplingFifoDirect(numCoefficients)
{
    if(numCoefficients % oversamplingFactor != 0){
        //确保系数个数能够被过采样倍数整除
        throw std::invalid_argument("numCoefficients must be divisible by oversamplingFactor");
    }
    setKernelSize();
    setMultiPhaseFIRCoefficientsMatrix();
    // upSamplingbuffer.resize(oversamplingFactor, 0.0f);
    upSamplingBufferMultiPhase.resize(oversamplingFactor, 0.0f);
    //内存分配绝对不能在单样本循环中进行，因为内存分配的开销会导致性能灾难

}

std::vector<float> OverSampling::processUpSamplingMultiPhase(float value){
    //每次处理一个输入样本，输出一个包含过采样倍数个样本的数组

    std::fill(upSamplingBufferMultiPhase.begin(), upSamplingBufferMultiPhase.end(), 0.0f);
    upSamplingFifoMultiPhase.push(value);
    for(int phase = 0; phase < oversamplingFactor; phase++){
        for(int index = 0; index < kernelSize; index++){
            upSamplingBufferMultiPhase[phase] += upSamplingFifoMultiPhase.buffer[
                getCircularBufferIndex(
                upSamplingFifoMultiPhase.read - index,
                //将历史缓冲区分别和四个相位的系数做差分方程
                
                upSamplingFifoMultiPhase.buffer.size())] * multiPhaseFIRCoefficientsMatrix[phase][index];
        }
    }
    return upSamplingBufferMultiPhase;
}

float OverSampling::processDownSamplingMultiPhase(std::vector<float>& inputBuffer){
    downSamplingFifoMultiPhase.pushBuffer(inputBuffer);
    float outputSample = 0.0f;
    for(int phase = 0; phase <oversamplingFactor; phase++){
        float phaseOutput = 0.0f;
        for(int index = 0; index < kernelSize; index++){
            phaseOutput += downSamplingFifoMultiPhase.buffer[
                getCircularBufferIndex(
                downSamplingFifoMultiPhase.read - index * oversamplingFactor - (oversamplingFactor - 1 - phase), 
                //read当前读取的位置一定是相位3,因为pushBuffer会将四个相位的数据依次推入缓冲区
                // 所以read指针每次移动4个位置，确保每个相位的数据都能被正确处理
                //- index * oversamplingFactor是为了实现抽取操作，即每隔oversamplingFactor个样本取一个样本进行处理
                //- (oversamplingFactor - 1 - phase)是为了对齐，如相位等于0,读取三个样本前的数据；
                // 相位等于1,读取两个样本前的数据；
                // 相位等于2,读取一个样本前的数据；
                // 相位等于3,读取当前的数据

                downSamplingFifoMultiPhase.buffer.size())] * multiPhaseFIRCoefficientsMatrix[phase][index];
        }
        outputSample += phaseOutput;
    }
    return outputSample;
}

void lowPassFilter::prepareToPlay(float dampLevel){
    a1 = -dampLevel;
    b0 = 1.0f - dampLevel;
}

void lowPassFilter::setValue(float dampLevel){
    prepareToPlay(dampLevel);
}

float lowPassFilter::processSample(float inputSample){
    float outputSample = b0 * inputSample - a1 * y1;
    y1 = outputSample;
    return outputSample;
}

void allPassFilterSingle::prepareToPlay(float sampleRate, float roomSize){
    delaySamplesNum = getNearestPrimeNumber(allPassDelayLineValue * sampleRate / defaultSampleRate * roomSize);
    writeIndex = 0;
}

void allPassFilterSingle::setValue(float sampleRate, float roomSize){
    delaySamplesNum = getNearestPrimeNumber(allPassDelayLineValue * sampleRate / defaultSampleRate * roomSize);
}

float allPassFilterSingle::processSample(float inputSample, float diffusion){
    int readIndex = getCircularBufferIndex(writeIndex - delaySamplesNum, static_cast<int>(allPassDelayLineBuffer.size()));
    float readSample = allPassDelayLineBuffer[readIndex];
    float outputSample = - diffusion * inputSample + readSample ;
    allPassDelayLineBuffer[writeIndex] = inputSample + diffusion * readSample;
    return outputSample;
}