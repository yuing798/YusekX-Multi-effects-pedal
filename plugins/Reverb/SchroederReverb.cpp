#include "SchroederReverb.h"
#include "constants.h"
#include "table.h"
#include "mathFunc.h"

SchroederReverbProcessor::SchroederReverbProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){}

//TODO2:Editor构造函数
SchroederReverbEditor::SchroederReverbEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts){
	addAndMakeVisible(mTitle);
	mTitle.setText("schroederReverb", juce::dontSendNotification);

    mOpenCloseButton.setClickingTogglesState(true);
	addAndMakeVisible(mOpenCloseButton);
	mOpenCloseButton.onClick = [this]{

        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
	};

    addAndMakeVisible(decayLevelLabel);
    decayLevelLabel.setText("Decay Level", juce::dontSendNotification);
    addAndMakeVisible(decayLevelSlider);

    addAndMakeVisible(diffusionLevelLabel);
    diffusionLevelLabel.setText("Diffusion Level", juce::dontSendNotification);
    addAndMakeVisible(diffusionLevelSlider);

    addAndMakeVisible(mixLevelLabel);
    mixLevelLabel.setText("Mix Level", juce::dontSendNotification);
    addAndMakeVisible(mixLevelSlider);

    addAndMakeVisible(dampLevelLabel);
    dampLevelLabel.setText("Damp Level", juce::dontSendNotification);
    addAndMakeVisible(dampLevelSlider);

    addAndMakeVisible(roomSizeLabel);
    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    addAndMakeVisible(roomSizeSlider);

    addAndMakeVisible(baseDelayTimeMsLabel);
    baseDelayTimeMsLabel.setText("Base Delay Time", juce::dontSendNotification);
    addAndMakeVisible(baseDelayTimeMsSlider);

    addAndMakeVisible(makeUpGainLabel);
    makeUpGainLabel.setText("Makeup Gain", juce::dontSendNotification);
    addAndMakeVisible(makeUpGainSlider);

    bindParameters();//将UI和APVTS参数绑定的函数放在构造函数中
}

//将ID和APVTS绑定
void SchroederReverbProcessor::createParameterLayout(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters)
{

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        SchroederReverbOpenId,
        "Schroeder Reverb Open",
        false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbDecayLevelId, 1 },
        "Schroeder Reverb Decay Level",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbDiffusionLevelId, 1 },
        "Schroeder Reverb Diffusion Level",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbMixLevelId, 1 },
        "Schroeder Reverb Mix Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbDampLevelId, 1 },
        "Schroeder Reverb Damp Level",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbRoomSizeId, 1 },
        "Schroeder Reverb Room Size",
        juce::NormalisableRange<float>(0.5f, 2.0f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbBaseDelayTimeMsId, 1 },
        "Schroeder Reverb Base Delay Time Ms",
        juce::NormalisableRange<float>(1.0f, 200.0f, 0.01f),
        10.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbMakeUpGainId, 1 },
        "Schroeder Reverb Make Up Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f));
    //......
}

//将UI滑块和APVTS参数绑定
void SchroederReverbEditor::bindParameters()
{
    mOpenCloseAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        SchroederReverbOpenId,
        mOpenCloseButton);
    decayLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbDecayLevelId,
        decayLevelSlider);
    diffusionLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbDiffusionLevelId,
        diffusionLevelSlider);
    mixLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbMixLevelId,
        mixLevelSlider);
    dampLevelAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbDampLevelId,
        dampLevelSlider);
    roomSizeAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbRoomSizeId,
        roomSizeSlider);
    baseDelayTimeMsAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbBaseDelayTimeMsId,
        baseDelayTimeMsSlider);
    makeUpGainAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbMakeUpGainId,
        makeUpGainSlider);

    //......

}

//组件位置:请务必全部模块使用绝对坐标系表示，不要使用removeFromRight和Reduced等布局函数
//UI布局只要能看就行了，不需要美观
void SchroederReverbEditor::resized()
{
    
    mTitle.setBounds(10,10,80,50);    
    mOpenCloseButton.setBounds(10, 70, 80, 30);    
    decayLevelLabel.setBounds(110, 10, 100, 30);
    decayLevelSlider.setBounds(220, 10, 150, 30);
    diffusionLevelLabel.setBounds(110, 50, 100, 30);
    diffusionLevelSlider.setBounds(220, 50, 150, 30);
    mixLevelLabel.setBounds(110, 90, 100, 30);
    mixLevelSlider.setBounds(220, 90, 150, 30);
    dampLevelLabel.setBounds(110, 130, 100, 30);
    dampLevelSlider.setBounds(220, 130, 150, 30);
    roomSizeLabel.setBounds(110, 170, 100, 30);
    roomSizeSlider.setBounds(220, 170, 150, 30);
    baseDelayTimeMsLabel.setBounds(110, 210, 100, 30);
    baseDelayTimeMsSlider.setBounds(220, 210, 150, 30);
    makeUpGainLabel.setBounds(110, 250, 100, 30);
    makeUpGainSlider.setBounds(220, 250, 150, 30);

}

//将DSP参数和APVTS参数同步
void SchroederReverbProcessor::syncParametersFromAPVTS()
{
    //if语句用来判断ID正确以及参数是否已经被注册
    if (auto* openParameter = mAPVTS.getRawParameterValue(SchroederReverbOpenId))
        isOpen = openParameter->load() >= 0.5f;

    if(auto* decayLevelParameter = mAPVTS.getRawParameterValue(SchroederReverbDecayLevelId))
        decayLevel = decayLevelParameter->load();
    if(auto* diffusionLevelParameter = mAPVTS.getRawParameterValue(SchroederReverbDiffusionLevelId))
        diffusionLevel = diffusionLevelParameter->load();
    if(auto* mixLevelParameter = mAPVTS.getRawParameterValue(SchroederReverbMixLevelId))
        mixLevel = mixLevelParameter->load();
    if(auto* dampLevelParameter = mAPVTS.getRawParameterValue(SchroederReverbDampLevelId))
        dampLevel = dampLevelParameter->load();
    if(auto* roomSizeParameter = mAPVTS.getRawParameterValue(SchroederReverbRoomSizeId))
        roomSize = roomSizeParameter->load();
    if(auto* baseDelayTimeMsParameter = mAPVTS.getRawParameterValue(SchroederReverbBaseDelayTimeMsId))
        baseDelayTimeMs = baseDelayTimeMsParameter->load();
    if(auto* makeUpGainParameter = mAPVTS.getRawParameterValue(SchroederReverbMakeUpGainId))
        makeUpGainDB = makeUpGainParameter->load();
}

//初始化
void SchroederReverbProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels)
{
    currentSampleRate = static_cast<float>(sampleRate);
    currentMaximumBlockSize = maximumBlockSize;


    //初始化同步
    syncParametersFromAPVTS();
    updateProcessorParameters();

    mSmoothedDecayLevel.reset(sampleRate, 0.02);
    mSmoothedDiffusionLevel.reset(sampleRate, 0.02);
    mSmoothedMixLevel.reset(sampleRate, 0.02);
    mSmoothedDampLevel.reset(sampleRate, 0.02);
    mSmoothedRoomSize.reset(sampleRate, 0.02);
    mSmoothedBaseDelayTimeMs.reset(sampleRate, 0.02);
    mSmoothedMakeUpGainDB.reset(sampleRate, 0.02);

    dryLookUpTable(dryTable, currentMaximumBlockSize);
    wetLookUpTable(wetTable, currentMaximumBlockSize);

    preDelayL.prepareToPlay(currentSampleRate);
    preDelayR.prepareToPlay(currentSampleRate);

    combFiltersL.prepareToPlay(currentSampleRate, roomSize, dampLevel);
    combFiltersR.prepareToPlay(currentSampleRate, roomSize, dampLevel);

    allPassFiltersL.prepareToPlay(currentSampleRate, roomSize);
    allPassFiltersR.prepareToPlay(currentSampleRate, roomSize);

    makeUpGain = std::pow(10.0f, makeUpGainDB / 20.0f);
}


void SchroederReverbProcessor::updateProcessorParameters()
{
    mSmoothedDecayLevel.setTargetValue(decayLevel);
    mSmoothedDiffusionLevel.setTargetValue(diffusionLevel);
    mSmoothedMixLevel.setTargetValue(mixLevel);
    mSmoothedDampLevel.setTargetValue(dampLevel);
    mSmoothedRoomSize.setTargetValue(roomSize);
    mSmoothedBaseDelayTimeMs.setTargetValue(baseDelayTimeMs);
    mSmoothedMakeUpGainDB.setTargetValue(makeUpGainDB);

}

//以下这个函数只做这四件事：同步参数，更新参数，判断按钮是否开启，进入正式执行函数
void SchroederReverbProcessor::processDelay(
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
void SchroederReverbProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    //执行链路
    //预延迟——》并联梳状滤波器——》低通滤波器——》串联全通滤波器——》干湿混合——》增益补偿

    auto *channelDataLeft = buffer.getWritePointer(0, startSample);
    float * channelDataRight = nullptr;
    if(numChannels > 1){
        channelDataRight = buffer.getWritePointer(1, startSample);
        //对右声道进行处理
    }
    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++){

        float currentDecayLevel = mSmoothedDecayLevel.getNextValue();
        float currentDiffusionLevel = mSmoothedDiffusionLevel.getNextValue();
        float currentMixLevel = mSmoothedMixLevel.getNextValue();
        float currentDampLevel = mSmoothedDampLevel.getNextValue();
        float currentRoomSize = mSmoothedRoomSize.getNextValue();
        float currentBaseDelayTimeMs = mSmoothedBaseDelayTimeMs.getNextValue();
        float currentMakeUpGainDB = mSmoothedMakeUpGainDB.getNextValue();

        float currentDry = dryTable[static_cast<int>(currentMixLevel * (dryTable.size() - 1))];
        float currentWet = wetTable[static_cast<int>(currentMixLevel * (wetTable.size() - 1))];

        if(mSmoothedBaseDelayTimeMs.isSmoothing()){
            preDelayL.setValue(currentSampleRate, currentBaseDelayTimeMs);
            preDelayR.setValue(currentSampleRate, currentBaseDelayTimeMs);
        }   
        if(mSmoothedDecayLevel.isSmoothing() || mSmoothedDampLevel.isSmoothing() || mSmoothedRoomSize.isSmoothing()){
            combFiltersL.setValue(currentSampleRate, currentRoomSize, currentDampLevel);
            combFiltersR.setValue(currentSampleRate, currentRoomSize, currentDampLevel);
        }
        if(mSmoothedDiffusionLevel.isSmoothing() || mSmoothedRoomSize.isSmoothing()){
            allPassFiltersL.setValue(currentSampleRate, currentRoomSize);
            allPassFiltersR.setValue(currentSampleRate, currentRoomSize);
        }
        if (mSmoothedMakeUpGainDB.isSmoothing())
            makeUpGain = std::pow(10.0f,currentMakeUpGainDB / 20.0f);


        float inputSampleLeft = channelDataLeft[sampleIndex];
        
        float drySampleLeft = inputSampleLeft;

        inputSampleLeft = preDelayL.processSample(inputSampleLeft);
        inputSampleLeft = combFiltersL.processSample(inputSampleLeft, currentDecayLevel);
        inputSampleLeft = allPassFiltersL.processSample(inputSampleLeft, currentDiffusionLevel);

        channelDataLeft[sampleIndex] = (inputSampleLeft * currentWet + drySampleLeft * currentDry) * makeUpGain;

        if(numChannels > 1){
            float inputSampleRight = channelDataRight[sampleIndex];
            float drySampleRight = inputSampleRight;

            inputSampleRight = preDelayR.processSample(inputSampleRight);
            inputSampleRight = combFiltersR.processSample(inputSampleRight, currentDecayLevel);
            inputSampleRight = allPassFiltersR.processSample(inputSampleRight, currentDiffusionLevel);

            channelDataRight[sampleIndex] = (inputSampleRight * currentWet + drySampleRight * currentDry) * makeUpGain;
        }
    }
}