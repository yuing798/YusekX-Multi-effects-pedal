#include "SchroederReverb.h"
#include "constants.h"


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

    addAndMakeVisible(dampHzLabel);
    dampHzLabel.setText("Damp Hz", juce::dontSendNotification);
    addAndMakeVisible(dampHzSlider);

    addAndMakeVisible(roomSizeLabel);
    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    addAndMakeVisible(roomSizeSlider);

    addAndMakeVisible(baseDelayTimeMsLabel);
    baseDelayTimeMsLabel.setText("Base Delay Time", juce::dontSendNotification);
    addAndMakeVisible(baseDelayTimeMsSlider);

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
        juce::ParameterID { SchroederReverbDampHzId, 1 },
        "Schroeder Reverb Damp Hz",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f),
        2000.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbRoomSizeId, 1 },
        "Schroeder Reverb Room Size",
        juce::NormalisableRange<float>(0.5f, 2.0f, 0.01f),
        0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { SchroederReverbBaseDelayTimeMsId, 1 },
        "Schroeder Reverb Base Delay Time Ms",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.01f),
        10.0f));

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
    dampHzAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbDampHzId,
        dampHzSlider);
    roomSizeAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbRoomSizeId,
        roomSizeSlider);
    baseDelayTimeMsAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        SchroederReverbBaseDelayTimeMsId,
        baseDelayTimeMsSlider);

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
    dampHzLabel.setBounds(110, 130, 100, 30);
    dampHzSlider.setBounds(220, 130, 150, 30);
    roomSizeLabel.setBounds(110, 170, 100, 30);
    roomSizeSlider.setBounds(220, 170, 150, 30);
    baseDelayTimeMsLabel.setBounds(110, 210, 100, 30);
    baseDelayTimeMsSlider.setBounds(220, 210, 150, 30);

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
    if(auto* dampHzParameter = mAPVTS.getRawParameterValue(SchroederReverbDampHzId))
        dampHz = dampHzParameter->load();
    if(auto* roomSizeParameter = mAPVTS.getRawParameterValue(SchroederReverbRoomSizeId))
        roomSize = roomSizeParameter->load();
    if(auto* baseDelayTimeMsParameter = mAPVTS.getRawParameterValue(SchroederReverbBaseDelayTimeMsId))
        baseDelayTimeMs = baseDelayTimeMsParameter->load();
}

//初始化
void SchroederReverbProcessor::prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels)
{
    currentSampleRate = sampleRate;


    //初始化同步
    syncParametersFromAPVTS();
    updateProcessorParameters();
}


void SchroederReverbProcessor::updateProcessorParameters()
{

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
    //该函数不要写，请留空，DSP部分我要自己写
}