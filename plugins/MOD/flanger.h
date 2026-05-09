#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "Utils/constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "dspFilters.h"

class FlangerEditor : public juce::Component
{
private:

    juce::Label title;
    juce::TextButton openCloseButton { "Open" };
    juce::TextButton feedbackPolarityButton { "+" };

    juce::Slider depthSlider;
    juce::Slider wetSlider;
    juce::Slider drySlider;
    juce::Slider rateSlider;
    juce::Slider feedbackSlider;
    juce::Slider baseDelaySlider;
    juce::Slider phaseOffsetSlider;
    juce::Slider feedbackDampSlider;
    juce::Slider lfoShapeSymmetrySlider;
    juce::Slider lfoShapeStruationSlider;

    juce::Label depthLabel;
    juce::Label wetLabel;
    juce::Label dryLabel;
    juce::Label rateLabel;
    juce::Label feedbackLabel;
    juce::Label baseDelayLabel;
    juce::Label phaseOffsetLabel;
    juce::Label feedbackDampLabel;
    juce::Label feedbackPolarityLabel;
    juce::Label lfoShapeSymmetryLabel;
    juce::Label lfoShapeStruationLabel;

    std::unique_ptr<ButtonAttachment> openCloseAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;
    std::unique_ptr<SliderAttachment> wetAttachment;
    std::unique_ptr<SliderAttachment> dryAttachment;
    std::unique_ptr<SliderAttachment> rateAttachment;
    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> baseDelayAttachment;
    std::unique_ptr<SliderAttachment> phaseOffsetAttachment;
    std::unique_ptr<SliderAttachment> feedbackDampAttachment;//反馈阻尼
    std::unique_ptr<ButtonAttachment> feedbackPolarityAttachment;//反馈极性
    std::unique_ptr<SliderAttachment> lfoShapeSymmetryAttachment;
    std::unique_ptr<SliderAttachment> lfoShapeStruationAttachment;

    juce::AudioProcessorValueTreeState& mAPVTS;

    void bindParameters();

public:
    explicit FlangerEditor(juce::AudioProcessorValueTreeState& apvts);
    ~FlangerEditor() override = default;
    void resized() override;
};

class FlangerProcessor
{
private:

    float depthMs { 4.0f };
    float rateHz { 0.35f };
    float wetLevel { 0.5f };
    float dryLevel{0.5f};
    float feedback { 0.0f };
    float baseDelayMs { 1.0f };
    float LFOPhaseOffset{180.0f};
    float feedbackDamp { 0.0f };
    float lfoShapeSymmetry{0.0f};
    float lfoShapeStruation{0.0f};


    double currentSampleRate { defaultSampleRate };
    int delayBufferLength { 0 };

    struct flangerState{
        float lfoPhase{0.0f};
        float writeIndex{0.0f};
        lowPassFilter lowpass;
        std::vector<float> wetDelayBuffer;
        preDelay preDelayState;
    };
    std::vector<flangerState> flangerStates;



    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        smoothOpenClose{1.0f},
        smoothDepth{depthMs},
        smoothRate{rateHz},
        smoothWet{wetLevel},
        smoothDry{dryLevel},
        smoothFeedback{feedback},
        smoothBaseDelay{baseDelayMs},
        smoothPhaseOffset{LFOPhaseOffset},
        smoothFeedbackDamp{feedbackDamp},
        smoothFeedbackPolarity{1.0f},
        smoothLfoShapeSymmetry{lfoShapeSymmetry},
        smoothLfoShapeStruation{lfoShapeStruation};


    juce::AudioProcessorValueTreeState& mAPVTS;


	void updateProcessorParameters();
    float processLFO(float phase, float symmetry, float saturation);

public:
    explicit FlangerProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~FlangerProcessor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void syncParametersFromAPVTS();


    void processFlanger(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};
