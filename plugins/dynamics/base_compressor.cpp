#include "base_compressor.h"

BaseCompressorProcessor::BaseCompressorProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){}

//TODO2:Editor构造函数
BaseCompressorEditor::BaseCompressorEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){

    mOpenCloseButton.setClickingTogglesState(true);
    addAndMakeVisible(mOpenCloseButton);//使得所有UI相关模块显现
	mOpenCloseButton.onClick = [this]{

        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
	};

    addAndMakeVisible(mTitle);
    mTitle.setText("Compressor", juce::dontSendNotification);

    addAndMakeVisible(thresoldDBLabel);
    thresoldDBLabel.setText("Thresold", juce::dontSendNotification);
    addAndMakeVisible(thresoldDBSlider);

    addAndMakeVisible(ratioLabel);
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    addAndMakeVisible(ratioSlider);

    addAndMakeVisible(attackTimeLabel);
    attackTimeLabel.setText("Attack Time", juce::dontSendNotification);
    addAndMakeVisible(attackTimeSlider);

    addAndMakeVisible(releaseTimeLabel);
    releaseTimeLabel.setText("Release Time", juce::dontSendNotification);
    addAndMakeVisible(releaseTimeSlider);

    addAndMakeVisible(makeupGainLabel);
    makeupGainLabel.setText("Makeup Gain", juce::dontSendNotification);
    addAndMakeVisible(makeupGainSlider);

    bindParameters();//将UI和APVTS参数绑定的函数放在构造函数中
}

//将ID和APVTS绑定
void BaseCompressorProcessor::createParameterLayout(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters)
{

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseCompressorOpenId, 1 },//ID名称请写在Utils/Constants文件中
        //如示例：static constexpr const char* BaseTremoloOpenId { "baseTremoloOpen" };
        "Open",//参数名称
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseCompressorThresoldId, 1 },
        "Thresold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -20.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseCompressorRatioId, 1 },
        "Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f , 0.1f),
        4.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseCompressorAttackTimeId, 1 },
        "Attack Time",
        juce::NormalisableRange<float>(0.1f, 200.0f, 0.1f),
        10.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseCompressorReleaseTimeId, 1 },
        "Release Time",
        juce::NormalisableRange<float>(10.0f, 2000.0f, 0.1f),
        100.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseCompressorMakeupGainId, 1 },
        "Makeup Gain",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        0.0f));
}

//将UI滑块和APVTS参数绑定
void BaseCompressorEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        BaseCompressorOpenId,
        mOpenCloseButton);
    thresoldDBAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseCompressorThresoldId,
        thresoldDBSlider);
    ratioAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseCompressorRatioId,
        ratioSlider);
    attackTimeAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseCompressorAttackTimeId,
        attackTimeSlider);
    releaseTimeAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseCompressorReleaseTimeId,
        releaseTimeSlider);
    makeupGainAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseCompressorMakeupGainId,
        makeupGainSlider); 

    //......

}

//组件位置:请务必全部模块使用绝对坐标系表示，不要使用removeFromRight和Reduced等布局函数
//UI布局只要能看就行了，不需要美观
void BaseCompressorEditor::resized()
{
    
    mTitle.setBounds(10,10,80,50);    
    mOpenCloseButton.setBounds(10, 70, 80, 30);    
    thresoldDBLabel.setBounds(10, 110, 80, 30);
    thresoldDBSlider.setBounds(100, 110, 150, 30);
    ratioLabel.setBounds(10, 150, 80, 30);
    ratioSlider.setBounds(100, 150, 150, 30);
    attackTimeLabel.setBounds(10, 190, 80, 30);
    attackTimeSlider.setBounds(100, 190, 150, 30);
    releaseTimeLabel.setBounds(10, 230, 80, 30);
    releaseTimeSlider.setBounds(100, 230, 150, 30);
    makeupGainLabel.setBounds(10, 270, 80, 30);
    makeupGainSlider.setBounds(100, 270, 150, 30);
}

//将DSP参数和APVTS参数同步
void BaseCompressorProcessor::syncParametersFromAPVTS()
{
    //if语句用来判断ID正确以及参数是否已经被注册
    if (auto* openParameter = mAPVTS.getRawParameterValue(BaseCompressorOpenId))
        isOpen = openParameter->load() >= 0.5f;

    if (auto* thresoldParameter = mAPVTS.getRawParameterValue(BaseCompressorThresoldId))
        thresoldDB = thresoldParameter->load();

    if (auto* ratioParameter = mAPVTS.getRawParameterValue(BaseCompressorRatioId))
        ratio = ratioParameter->load();

    if (auto* attackTimeParameter = mAPVTS.getRawParameterValue(BaseCompressorAttackTimeId))
        attackTimeMs = attackTimeParameter->load();

    if (auto* releaseTimeParameter = mAPVTS.getRawParameterValue(BaseCompressorReleaseTimeId))
        releaseTimeMs = releaseTimeParameter->load();

    if (auto* makeupGainParameter = mAPVTS.getRawParameterValue(BaseCompressorMakeupGainId))
        makeupGainDB = makeupGainParameter->load();

}

//初始化
void BaseCompressorProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels)
{
    currentSampleRate = sampleRate;


    //初始化同步
    syncParametersFromAPVTS();
    updateProcessorParameters();

    mSmoothedThresoldDB.reset(currentSampleRate, 0.02);
    mSmoothedRatio.reset(currentSampleRate, 0.02);
    mSmoothedAttackTimeMs.reset(currentSampleRate, 0.02);
    mSmoothedReleaseTimeMs.reset(currentSampleRate, 0.02);
    mSmoothedMakeupGainDB.reset(currentSampleRate, 0.02);

    attackAndReleaseLeft.setValue(currentSampleRate, attackTimeMs);
    attackAndReleaseRight.setValue(currentSampleRate, attackTimeMs);
}


void BaseCompressorProcessor::updateProcessorParameters()
{
    // 更新压缩器参数
    mSmoothedThresoldDB.setTargetValue(thresoldDB);
    mSmoothedRatio.setTargetValue(ratio);
    mSmoothedAttackTimeMs.setTargetValue(attackTimeMs);
    mSmoothedReleaseTimeMs.setTargetValue(releaseTimeMs);
    mSmoothedMakeupGainDB.setTargetValue(makeupGainDB);
}

//以下这个函数只做这四件事：同步参数，更新参数，判断按钮是否开启，进入正式执行函数
void BaseCompressorProcessor::processCompressor(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{

    syncParametersFromAPVTS();
    updateProcessorParameters();//平滑度更新不用放在for循环中

    if (!isOpen)
        return;

    processBlock(buffer, startSample, numSamples, numChannels);
}

//核心效果器处理部分
void BaseCompressorProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    auto *channelDataLeft = buffer.getWritePointer(0, startSample);
    float * channelDataRight = nullptr;
    if(numChannels > 1){
        channelDataRight = buffer.getWritePointer(1, startSample);
        //对右声道进行处理
    }
    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++){

        float currentThresoldDB = mSmoothedThresoldDB.getNextValue();
        float currentRatio = mSmoothedRatio.getNextValue();
        float currentAttackTimeMs = mSmoothedAttackTimeMs.getNextValue();
        float currentReleaseTimeMs = mSmoothedReleaseTimeMs.getNextValue();
        float currentMakeupGainDB = mSmoothedMakeupGainDB.getNextValue();

        float inputSampleLeft = channelDataLeft[sampleIndex];
        //对左声道进行处理，处理完后写回channelDataLeft[sampleIndex]

        //转换为dB，1e-6f避免log(0)导致的负无穷大
        float inputSampleLeftDB = juce::Decibels::gainToDecibels(std::abs(inputSampleLeft) + 1e-6f);
        float gainLeftDB = 0.0f;

        float slope = 1.0f - 1.0f / currentRatio;
        float over = inputSampleLeftDB - currentThresoldDB;
        float k = over + kneeRangeDB * 0.5f;//中间系数，方便计算，没有物理意义
        if(over > kneeRangeDB * 0.5f){
            //(targetDB - thresoldDB) / (inputDB - thresoldDB) = 1 / ratio
            gainLeftDB =  - over * slope;
        } else if(over > -kneeRangeDB * 0.5f){
            //在 $over = -W/2$ 处，它的值是 $0$，且斜率也是 $0$（完美衔接不压缩区域）。
            // 在 $over = W/2$ 处，它的值和斜率都与直线压缩区域完全相等。
            gainLeftDB = - slope * k * k / (2 * kneeRangeDB);
        }

        if(gainLeftDB < attackAndReleaseLeft.y1){//和前一个采样值比较进行逻辑判断，而不是和阈值进行判断
            if(mSmoothedAttackTimeMs.isSmoothing())
                attackAndReleaseLeft.setValue(currentSampleRate, currentAttackTimeMs);
        } else{
            if(mSmoothedReleaseTimeMs.isSmoothing())
                attackAndReleaseLeft.setValue(currentSampleRate, currentReleaseTimeMs);
        }//这个if语句要放在“增益计算 (Gain Computer)：计算如果不考虑平滑，理论上应该压掉多少 dB”之后

        gainLeftDB = attackAndReleaseLeft.processSample(gainLeftDB);

        //将处理后的dB值转换回线性增益
        float gainLeft = juce::Decibels::decibelsToGain(gainLeftDB + currentMakeupGainDB) ;

        channelDataLeft[sampleIndex] = inputSampleLeft * gainLeft;

        if(numChannels > 1){
            float inputSampleRight = channelDataRight[sampleIndex];
            //对右声道进行处理，处理完后写回channelDataRight[sampleIndex]

            //转换为dB，1e-6f避免log(0)导致的负无穷大
            float inputSampleRightDB = juce::Decibels::gainToDecibels(std::abs(inputSampleRight) + 1e-6f);
            float gainRightDB = 0.0f;

            float slopeRight = 1.0f - 1.0f / currentRatio;
            float overRight = inputSampleRightDB - currentThresoldDB;
            float kRight = overRight + kneeRangeDB * 0.5f;//中间系数，方便计算，没有物理意义
            if(overRight > kneeRangeDB * 0.5f){
                gainRightDB = - overRight * slopeRight;
            } else if(overRight > -kneeRangeDB * 0.5f){
                gainRightDB = - slopeRight * kRight * kRight / (2 * kneeRangeDB);
            }

            if(gainRightDB < attackAndReleaseRight.y1){//和前一个采样值比较进行逻辑判断，而不是和阈值进行判断
                if(mSmoothedAttackTimeMs.isSmoothing())
                    attackAndReleaseRight.setValue(currentSampleRate, currentAttackTimeMs);
            } else{
                if(mSmoothedReleaseTimeMs.isSmoothing())
                    attackAndReleaseRight.setValue(currentSampleRate, currentReleaseTimeMs);
            }//这个if语句要放在“增益计算 (Gain Computer)：计算如果不考虑平滑，理论上应该压掉多少 dB”之后

            gainRightDB = attackAndReleaseRight.processSample(gainRightDB);

            //将处理后的dB值转换回线性增益
            float gainRight = juce::Decibels::decibelsToGain(gainRightDB + currentMakeupGainDB) ;

            channelDataRight[sampleIndex] = inputSampleRight * gainRight;
        }
    }
}