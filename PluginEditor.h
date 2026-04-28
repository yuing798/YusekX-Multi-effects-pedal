#pragma once
//该文件定义插件的用户界面类，包含了插件的外观和交互功能实现
#include "PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "plugins/Booster/base_overdrive.h"
#include "plugins/Delay/base_delay.h"
#include "plugins/EQ/eq.h"
#include "plugins/MOD/multiChannelsChorus.h"
#include "plugins/Reverb/SchroederReverb.h"
#include "plugins/Tremolo/base_tremolo.h"
#include "plugins/dynamics/base_compressor.h"
#include "plugins/plugins.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    //禁止隐式转换
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //Attachment在底层直接实现了回调函数的功能，绑定了UI组件和APVTS参数之间的关系
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> 
        outputGainAttachment;

    juce::Slider outputGainSlider;

    AudioPluginAudioProcessor& processorRef;

    juce::TextButton testButton { "midi test on" };

    juce::ComboBox effectSelector;



    BaseDelayEditor mBaseDelayEditor;
    BaseTremoloEditor mBaseTremoloEditor;
    SineSurroundEditor mSineSurroundEditor;
    YOK3508Editor mYOK3508Editor;
    baseOverdriveEditor mBaseOverdriveEditor;
    baseEQEditor mBaseEQEditor;
    BaseCompressorEditor mBaseCompressorEditor;
    SchroederReverbEditor mSchroederReverbEditor;

    void updateEffectEditor();
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
