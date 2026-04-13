#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils/mathFunc.h"
#include "juce_audio_processors_headless/juce_audio_processors_headless.h"
#include "plugins/Delay/base_delay.h"
#include <vector>

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
        #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        #endif
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        #endif
        ),
        apvts(*this, //apvts的生命周期和插件处理器绑定
            nullptr, //撤销记录管理器
            "Parameters", //根节点标签名
            createParameterLayout()),
            mBaseDelayProcessor(apvts)
            //mBaseTremoloProcessor(apvts)
                        
{
    mMidiInfo.sineTable.clear();
    SineLookUpTable(mMidiInfo.sineTable, bufferSize);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
//提供参数清单
juce::AudioProcessorValueTreeState::ParameterLayout
    AudioPluginAudioProcessor::createParameterLayout()
{
    //vector里面只是一定要存放unique_ptr，
    // 参数的实际类型可以是AudioParameterFloat、AudioParameterBool等不同类型
    //std::unique_ptr是模板化函数
    //juce::AudioParameterFloat，juce::AudioParameterBool等
    // 都是继承自juce::RangedAudioParameter的类
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "outputGain", 1 },//参数ID,版本号
        "Output Gain",//参数名称
        juce::NormalisableRange<float> 
            (-60.0f, 
            6.0f, 
            0.1f),//参数范围和步长
        0.0f,//默认值
        "dB"));//参数标签

    BaseDelayProcessor::createParameterLayout(parameters);
    //BaseTremoloProcessor::createParameterLayout(parameters);

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    mCurrentSampleRate = sampleRate; //保存当前采样率

    mMidiInfo.midiGain.reset(sampleRate, 0.1); //设置平滑器的采样率和时间常数
    mMidiInfo.midiGain.setCurrentAndTargetValue(0.0f); //初始化平滑器的当前值和目标值

    mBaseDelayProcessor.prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    //mBaseTremoloProcessor.prepareToPlay(sampleRate);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}


//核心音频流函数 ！ ！ ！
void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    
    auto numSamples = buffer.getNumSamples();

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //这个地方在存储数据，不改变实际听感
    //这个变量的作用和base_delay.cpp中的syncParametersFromAPVTS函数的作用相同，用来获取APVTS参数
    auto gainDB = 
        apvts.getRawParameterValue("outputGain");
    auto gainLinear = 
        juce::Decibels::decibelsToGain(gainDB->load());

    if (isMidiTestOn.load())
    {
        auto testMidiPlus = mMidiInfo.testMidiInfo(
            midiMessages, 
            numSamples, 
            mCurrentSampleRate);
        for(int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] += testMidiPlus[sample];
            }
        }
    }

    mBaseDelayProcessor.processDelay(
        buffer, 
        0, 
        numSamples, 
        totalNumOutputChannels);

    //mBaseTremoloProcessor.processTremolo(
    //    buffer,
    //    0,
    //    numSamples,
    //    totalNumOutputChannels);

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        buffer.applyGain(channel, 0, numSamples, gainLinear);
        
    }

}

//这里他妈是我为了测试声音写的他妈的简单midi声源
std::vector<float> AudioPluginAudioProcessor::mMidiInfo::testMidiInfo(
    juce::MidiBuffer& midiMessages, 
    int numSamples, 
    double sampleRate)
{
    for (const auto& midiMessage : midiMessages)
    {
        const auto message = midiMessage.getMessage();

        if (message.isNoteOn())
        {           
            isNoteOn = true;
            currentIndex = 0.0f; //重置振荡器的相位索引
            noteNumber = message.getNoteNumber();
            velocity = message.getFloatVelocity(); //归一化力度
            midiGain.setTargetValue(
                velocity); //将力度值设置为平滑器的目标值

            auto frequency = juce::MidiMessage::getMidiNoteInHertz(             
                noteNumber);
            phaseStep = static_cast<float>(frequency / sampleRate); //计算相位步长
        }
        else if (message.isNoteOff())
        {
            isNoteOn = false;
            midiGain.setTargetValue(0.0f); //当音符关闭时，将目标值设置为0以实现平滑衰减
        }
    }
    std::vector<float> testMidiPlus(numSamples, 0.0f);//测试midi的输出应该直接加在通道上而不是覆盖原有的音频信号
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        
        const auto currentGain = midiGain.getNextValue(); //获取当前平滑增益值

        if (isNoteOn || currentGain > 0.001f) 
        {
            // indexStep/tableSize == phaseStep / 1
            const auto indexStep = 
                phaseStep * sineTable.size();

            //线性插值获取正弦波表的值
            const auto sineValue = 
                getLinearInterpolator(sineTable.data(),
                    static_cast<int>(sineTable.size()),
                    currentIndex);

            //更新当前索引，使用循环缓冲区索引函数确保索引在表的范围内
            currentIndex = getCircularBufferIndex(
                currentIndex + indexStep, 
                static_cast<int>(sineTable.size()));

            const auto smoothedVelocity = currentGain; //使用平滑增益值控制音量
            testMidiPlus[sample] = sineValue * smoothedVelocity;
        }
        else{
            testMidiPlus[sample] = 0.0f;
        }
    }
    return testMidiPlus;
}


//==============================================================================
//当用户保存 DAW 工程或保存预设时调用，你应该在这里将插件的参数状态保存到 destData 中
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

//加载缓存的插件参数状态，当用户加载 DAW 工程或预设时调用，你应该在这里从 data 中恢复插件的参数状态
void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (
        getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (
        apvts.state.getType())){
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
        
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

//==============================================================================
bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}
