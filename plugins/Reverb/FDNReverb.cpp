#include "FDNReverb.h"
#include "constants.h"
#include "table.h"
#include "mathFunc.h"

FDNReverbProcessor::FDNReverbProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){
    duckerProcessor = std::make_unique<DuckerProcessor>(mAPVTS);    
}

//TODO2:Editor构造函数
FDNReverbEditor::FDNReverbEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){
	addAndMakeVisible(mTitle);
	mTitle.setText("fdnReverb", juce::dontSendNotification);

    mOpenCloseButton.setClickingTogglesState(true);
	addAndMakeVisible(mOpenCloseButton);
	mOpenCloseButton.onClick = [this]{

        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
	};

    addAndMakeVisible(decayLevelLabel);
    decayLevelLabel.setText("Decay Level", juce::dontSendNotification);
    addAndMakeVisible(decayLevelSlider);

    addAndMakeVisible(dryLevelLabel);
    dryLevelLabel.setText("Dry Level", juce::dontSendNotification);
    addAndMakeVisible(dryLevelSlider);

    addAndMakeVisible(wetLevelLabel);
    wetLevelLabel.setText("Wet Level", juce::dontSendNotification);
    addAndMakeVisible(wetLevelSlider);

    addAndMakeVisible(dampLevelLabel);
    dampLevelLabel.setText("Damp Level", juce::dontSendNotification);
    addAndMakeVisible(dampLevelSlider);

    addAndMakeVisible(roomSizeLabel);
    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    addAndMakeVisible(roomSizeSlider);

    addAndMakeVisible(preDelayTimeMsLabel);
    preDelayTimeMsLabel.setText("Pre-Delay Time", juce::dontSendNotification);
    addAndMakeVisible(preDelayTimeMsSlider);

    addAndMakeVisible(makeUpGainLabel);
    makeUpGainLabel.setText("Makeup Gain", juce::dontSendNotification);
    addAndMakeVisible(makeUpGainSlider);

    duckerEditor = std::make_unique<DuckerEditor>(mAPVTS);
    addAndMakeVisible(*duckerEditor);
    duckerEditor->makeDuckerVisible();

    bindParameters();//将UI和APVTS参数绑定的函数放在构造函数中
}

//将ID和APVTS绑定
void FDNReverbProcessor::createParameterLayout(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters)
{

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        FDNReverbOpenId,
        "FDN Reverb Open",
        false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { FDNReverbDecayLevelId, 1 },
        "FDN Reverb Decay Level",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { FDNReverbDryLevelId, 1 },
        "FDN Reverb Dry Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { FDNReverbWetLevelId, 1 },
        "FDN Reverb Wet Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { FDNReverbDampLevelId, 1 },
        "FDN Reverb Damp Level",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { FDNReverbRoomSizeId, 1 },
        "FDN Reverb Room Size",
        juce::NormalisableRange<float>(0.5f, 2.0f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { FDNReverbPreDelayTimeMsId, 1 },
        "FDN Reverb PreDelay Time Ms",
        juce::NormalisableRange<float>(1.0f, 200.0f, 0.01f),
        10.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { FDNReverbMakeUpGainId, 1 },
        "FDN Reverb Make Up Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f));

    DuckerProcessor::createParameterLayout(parameters, FDNReverbDuckerOpenId, FDNReverbDuckerModeId);
    //......
}

//将UI滑块和APVTS参数绑定
void FDNReverbEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        FDNReverbOpenId,
        mOpenCloseButton);
    decayLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        FDNReverbDecayLevelId,
        decayLevelSlider);
    dryLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        FDNReverbDryLevelId,
        dryLevelSlider);
    wetLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        FDNReverbWetLevelId,
        wetLevelSlider);
    dampLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        FDNReverbDampLevelId,
        dampLevelSlider);
    roomSizeAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        FDNReverbRoomSizeId,
        roomSizeSlider);
    preDelayTimeMsAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        FDNReverbPreDelayTimeMsId,
        preDelayTimeMsSlider);
    makeUpGainAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        FDNReverbMakeUpGainId,
        makeUpGainSlider);
    duckerEditor->bindParameters(FDNReverbDuckerOpenId, FDNReverbDuckerModeId);

    //......

}

//组件位置:请务必全部模块使用绝对坐标系表示，不要使用removeFromRight和Reduced等布局函数
//UI布局只要能看就行了，不需要美观
void FDNReverbEditor::resized()
{
    
    mTitle.setBounds(10,10,80,50);    
    mOpenCloseButton.setBounds(10, 70, 80, 30);    
    decayLevelLabel.setBounds(110, 10, 100, 30);
    decayLevelSlider.setBounds(220, 10, 150, 30);
    dryLevelLabel.setBounds(110, 50, 100, 30);
    dryLevelSlider.setBounds(220, 50, 150, 30);
    wetLevelLabel.setBounds(110, 90, 100, 30);
    wetLevelSlider.setBounds(220, 90, 150, 30);
    dampLevelLabel.setBounds(110, 130, 100, 30);
    dampLevelSlider.setBounds(220, 130, 150, 30);
    roomSizeLabel.setBounds(110, 170, 100, 30);
    roomSizeSlider.setBounds(220, 170, 150, 30);
    preDelayTimeMsLabel.setBounds(110, 210, 100, 30);
    preDelayTimeMsSlider.setBounds(220, 210, 150, 30);
    makeUpGainLabel.setBounds(110, 250, 100, 30);
    makeUpGainSlider.setBounds(220, 250, 150, 30);

    duckerEditor->setBounds(110, 290, 260, 50);

}

//将DSP参数和APVTS参数同步
void FDNReverbProcessor::syncParametersFromAPVTS()
{
    //if语句用来判断ID正确以及参数是否已经被注册
    if (auto* openParameter = mAPVTS.getRawParameterValue(FDNReverbOpenId))
        isOpen = openParameter->load() >= 0.5f;

    if(auto* decayLevelParameter = mAPVTS.getRawParameterValue(FDNReverbDecayLevelId))
        decayLevel = decayLevelParameter->load();
    if(auto* dryLevelParameter = mAPVTS.getRawParameterValue(FDNReverbDryLevelId))
        dryLevel = dryLevelParameter->load();
    if(auto* wetLevelParameter = mAPVTS.getRawParameterValue(FDNReverbWetLevelId))
        wetLevel = wetLevelParameter->load();
    if(auto* dampLevelParameter = mAPVTS.getRawParameterValue(FDNReverbDampLevelId))
        dampLevel = dampLevelParameter->load();
    if(auto* roomSizeParameter = mAPVTS.getRawParameterValue(FDNReverbRoomSizeId))
        roomSize = roomSizeParameter->load();
    if(auto* preDelayTimeMsParameter = mAPVTS.getRawParameterValue(FDNReverbPreDelayTimeMsId))
        preDelayTimeMs = preDelayTimeMsParameter->load();
    if(auto* makeUpGainParameter = mAPVTS.getRawParameterValue(FDNReverbMakeUpGainId))
        makeUpGainDB = makeUpGainParameter->load();

    duckerProcessor->syncParametersFromAPVTS(FDNReverbDuckerOpenId, FDNReverbDuckerModeId, currentSampleRate);
}

//初始化
void FDNReverbProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels)
{
    currentSampleRate = static_cast<float>(sampleRate);
    currentMaximumBlockSize = maximumBlockSize;


    //初始化同步
    syncParametersFromAPVTS();
    updateProcessorParameters();

    mSmoothedDecayLevel.reset(sampleRate, 0.02);
    mSmoothedDryLevel.reset(sampleRate, 0.02);
    mSmoothedWetLevel.reset(sampleRate, 0.02);
    mSmoothedDampLevel.reset(sampleRate, 0.02);
    mSmoothedRoomSize.reset(sampleRate, 0.02);
    mSmoothedPreDelayTimeMs.reset(sampleRate, 0.02);
    mSmoothedMakeUpGainDB.reset(sampleRate, 0.02);

    for(int channel = 0; channel < numChannels; ++channel){
        preDelays[channel].prepareToPlay(200.0f, currentSampleRate);
        FDNNetworks[channel].prepareToPlay(currentSampleRate, roomSize, dampLevel);
    }

    makeUpGain = std::pow(10.0f, makeUpGainDB / 20.0f);

    duckerProcessor->prepareToPlay(sampleRate); 
}


void FDNReverbProcessor::updateProcessorParameters()
{
    mSmoothedDecayLevel.setTargetValue(decayLevel);
    mSmoothedDryLevel.setTargetValue(dryLevel);
    mSmoothedWetLevel.setTargetValue(wetLevel);
    mSmoothedDampLevel.setTargetValue(dampLevel);
    mSmoothedRoomSize.setTargetValue(roomSize);
    mSmoothedPreDelayTimeMs.setTargetValue(preDelayTimeMs);
    mSmoothedMakeUpGainDB.setTargetValue(makeUpGainDB);

}

//以下这个函数只做这四件事：同步参数，更新参数，判断按钮是否开启，进入正式执行函数
void FDNReverbProcessor::processDelay(
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
void FDNReverbProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    //执行链路
    //预延迟——》并联梳状滤波器——》低通滤波器——》串联全通滤波器——》干湿混合——》增益补偿

    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++){

        float currentDecayLevel = mSmoothedDecayLevel.getNextValue();
        float currentDry = mSmoothedDryLevel.getNextValue();
        float currentWet = mSmoothedWetLevel.getNextValue();
        float currentDampLevel = mSmoothedDampLevel.getNextValue();
        float currentRoomSize = mSmoothedRoomSize.getNextValue();
        float currentPreDelayTimeMs = mSmoothedPreDelayTimeMs.getNextValue();
        float currentMakeUpGainDB = mSmoothedMakeUpGainDB.getNextValue();


        if (mSmoothedMakeUpGainDB.isSmoothing())
            makeUpGain = std::pow(10.0f,currentMakeUpGainDB / 20.0f);

        for(int channel = 0; channel < numChannels; ++channel){

            if(mSmoothedPreDelayTimeMs.isSmoothing()){
                preDelays[channel].setValue(currentSampleRate, currentPreDelayTimeMs);
            }   
            if(mSmoothedDecayLevel.isSmoothing() || mSmoothedDampLevel.isSmoothing() || mSmoothedRoomSize.isSmoothing()){
                FDNNetworks[channel].setValue(currentSampleRate, currentRoomSize, currentDampLevel);
            }

            float* channelData = buffer.getWritePointer(channel, startSample);
            float inputSample = channelData[sampleIndex];
            float drySample = inputSample * currentDry;
            float duckerGain = duckerProcessor->processSample(inputSample, duckerProcessor->attackAndReleaseFilters[channel]);
            inputSample = preDelays[channel].processSample(inputSample);
            inputSample = FDNNetworks[channel].processSample(inputSample, currentDecayLevel);
            channelData[sampleIndex] = (inputSample * currentWet * duckerGain + drySample) * makeUpGain;
        }
    }
}