#pragma once
#include <JuceHeader.h>
#include "constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>


class DuckerEditor : public juce::Component{
private:
    juce::TextButton duckerOpenCloseButton { "Ducker Open" };
    juce::ComboBox duckerModeComboBox;

    std::unique_ptr<ComboBoxAttachment> duckerModeAttachment;
    std::unique_ptr<ButtonAttachment> duckerOpenAttachment;

    juce::AudioProcessorValueTreeState& mAPVTS;

public:
    explicit DuckerEditor(juce::AudioProcessorValueTreeState& apvts);
    ~DuckerEditor() override = default;
    void resized() override;
    void makeDuckerVisible();
    void bindParameters(std::string DuckerOpenId, std::string DuckerModeId);
};
struct DuckerProcessor
{


    bool isOpen { false };
    int duckerMode { 1 };//默认模式为Light
    float thresoldDB { -24.0f };
    float ratio { 4.0f };
    float attackTimeMS { 2.0f };
    float releaseTimeMS { 150.0f };
    float maxAttenuationDB { -15.0f };//最大衰减量

    juce::AudioProcessorValueTreeState& mAPVTS;

    struct attackAndReleaseFilter{
        float b0{0.0f};
        float b1{0.0f};//分子
        float a0{1.0f};//分母

        float y1{0.0f};//上一个输出样本

        float attackAlpha{0.0f};//攻击时间对应的alpha值
        float releaseAlpha{0.0f};//释放时间对应的alpha值

        void setAttackAlpha(float sampleRate, float attackTimeMs){
            attackAlpha = 1 - std::exp(-1 / (sampleRate * attackTimeMs * 0.001f));
        }

        void setReleaseAlpha(float sampleRate, float releaseTimeMs){    
            releaseAlpha = 1 - std::exp(-1 / (sampleRate * releaseTimeMs * 0.001f));
        }

        void setValue(float currentAlpha){
            b1 = 1 - currentAlpha;
            a0 = currentAlpha;
        }

        float processSample(float inputSample){
            float outputSample = b1 * y1 + a0 * inputSample;
            y1 = outputSample;
            return outputSample;
        }
    };

    std::vector<attackAndReleaseFilter> attackAndReleaseFilters{2};

    float kneeRangeDB { 10.0f };

    explicit DuckerProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~DuckerProcessor() = default;

    void updateParameters(int selectedId);

    static void createParameterLayout(
        std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters,
        std::string DuckerOpenId,
        std::string DuckerModeId);

    void syncParametersFromAPVTS(std::string DuckerOpenId, std::string DuckerModeId, float sampleRate);

    float processSample(float drySample, attackAndReleaseFilter& attackAndRelease);

    void prepareToPlay(float sampleRate);

    void setUpdateValue(float sampleRate);
};

struct SharedMetronomeStates{//节拍器
    juce::AudioBuffer<float> stressBuffer;//重音音色缓冲包
    juce::AudioBuffer<float> lightBuffer;//轻音音色缓冲包
    float sampleRate{defaultSampleRate};
};

class metronomeMachineEditor : public juce::Component
{
private:

    juce::TextButton openCloseButtom;

    juce::Label gainDBLabel;//节拍器自己管理的增益（不和实际音频流音量同步）
    juce::Label bpmLabel;//bpm大小
    juce::Label stressSeletorLabel;//重音音色选择
    juce::Label lightSeletorLabel;//轻音音色选择

    juce::Slider gainDBSlider;
    juce::Slider bpmSlider;
    juce::ComboBox stressSeletorComboBox;
    juce::ComboBox lightSeletorComboBox;

    std::unique_ptr<SliderAttachment> gainDBAttachment;
    std::unique_ptr<SliderAttachment> bpmAttachment;
    std::unique_ptr<ComboBoxAttachment> stressSeletorAttachment;
    std::unique_ptr<ComboBoxAttachment> lightSeletorAttachment; 

    std::atomic<SharedMetronomeStates*> currentDataPtr{nullptr};

public:
    void loadAudioFile(const juce::File& file);//将音色注册到程序中
    void resized() override;//组件布局
    void setUIVisible();//使得组件可视化
    void updateStressTone();//切换重音音色
    void updateLightTone();//设置轻音音色

};
class metronomeMachineProcessor{

private:

    float bpm{0.0f};
    float gainDB{0.0f};
    int count{0};//节拍器的计数器
    std::atomic<bool> isOpen;

    std::atomic<SharedMetronomeStates*> currentDataPtr{nullptr};

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedBpm,smoothedGainDB;
    //音色切换在节拍器停止后才设置，所以没必要使用平滑

public:

    void transformBufferState(float sampleRate, int numChannels);
    //音色文件的采样率和通道数都可能和实际音频流不同，需要根据实际音频流状态进行更新
    void updateStressTone();//切换重音音色
    void updateLightTone();//设置轻音音色
};