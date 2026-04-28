#pragma once
//该文件定义逻辑和声音处理类，包含了插件的核心功能实现
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include "Utils/constants.h"
#include "Utils/mathFunc.h"
#include "Utils/table.h"
#include "plugins/Booster/base_overdrive.h"
#include "plugins/Delay/base_delay.h"
#include "plugins/Delay/sine_surround.h"
#include "plugins/EQ/eq.h"
#include "plugins/MOD/multiChannelsChorus.h"
#include "plugins/Reverb/SchroederReverb.h"
#include "plugins/Tremolo/base_tremolo.h"
#include "plugins/plugins.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    //告诉 DAW 支持什么样的声道配置（单声道/立体声）
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    //using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override {return JucePlugin_Name;}

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    //告诉 DAW 效果是否有“尾巴”（如延时或混响）
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    std::atomic<bool> isMidiTestOn{false};//用于测试 MIDI 声源的开关

private:
    //==============================================================================
    double mCurrentSampleRate{defaultSampleRate}; //当前采样率

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    struct mMidiInfo
    {
        bool isNoteOn{ false };
        int noteNumber{ 0 };
        float velocity{ 0.0f };// MIDI 音符的力度
        float phaseStep{ 0.0f };// 用于振荡器的相位步长
        float currentIndex{ 0.0f };
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> midiGain;
        

        std::vector<float> testMidiInfo(juce::MidiBuffer& midiMessages, 
            int numSamples, 
            double sampleRate,
            std::vector<float> sineTable
        );
    } mMidiInfo;

    struct table{
        std::vector<float> sineTable;
        std::vector<float> cosTable;

        void initSineTable(std::vector<float>& sineTable, int tableSize) {
            SineLookUpTable(sineTable, tableSize);
        }
        void initCosTable(std::vector<float>& cosTable, int tableSize) {
            CosLookUpTable(cosTable, tableSize);
        }

    }table;//把查询表都放在主函数初始化

    BaseDelayProcessor mBaseDelayProcessor;
    BaseTremoloProcessor mBaseTremoloProcessor;
    SineSurroundProcessor mSineSurroundProcessor;
    YOK3508Processor mYOK3508Processor;
    baseOverdriveProcessor  mBaseOverdriveProcessor;
    baseEQProcessor mBaseEQProcessor;
    BaseCompressorProcessor mBaseCompressorProcessor;
    SchroederReverbProcessor mSchroederReverbProcessor;

    float processTime{ 0.0f };
    int processCount{ 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
