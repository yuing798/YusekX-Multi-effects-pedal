#pragma once

#include "constants.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <memory>
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