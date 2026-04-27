#include <JuceHeader.h>
#include "Utils/constants.h"
#include "juce_gui_basics/juce_gui_basics.h"


class SchroederReverbEditor final : public juce::Component
{
private:

	juce::Label mTitle;
	juce::TextButton mOpenCloseButton { "Open" };

	juce::Slider decayLevelSlider;//反馈梳状滤波器反馈度
    juce::Slider diffusionLevelSlider;//扩散全通滤波器的扩散度
    juce::Slider mixLevelSlider;//干湿混合度
    juce::Slider dampHzSlider;//低通滤波器的截止频率
    juce::Slider roomSizeSlider;//房间尺寸，影响延迟时间
    juce::Slider baseDelayTimeMsSlider;//基础延迟时间，影响初始反射的时间
    juce::Slider makeUpGainSlider;//补偿增益

	juce::Label decayLevelLabel;
    juce::Label diffusionLevelLabel;
    juce::Label mixLevelLabel;
    juce::Label dampHzLabel;
    juce::Label roomSizeLabel;
    juce::Label baseDelayTimeMsLabel;
    juce::Label makeUpGainLabel;

	std::unique_ptr<ButtonAttachment> mOpenCloseAttachment;
    std::unique_ptr<SliderAttachment> decayLevelAttachment;
    std::unique_ptr<SliderAttachment> diffusionLevelAttachment;
    std::unique_ptr<SliderAttachment> mixLevelAttachment;
    std::unique_ptr<SliderAttachment> dampHzAttachment;
    std::unique_ptr<SliderAttachment> roomSizeAttachment;
    std::unique_ptr<SliderAttachment> baseDelayTimeMsAttachment;
    std::unique_ptr<SliderAttachment> makeUpGainAttachment;

	void bindParameters();

    juce::AudioProcessorValueTreeState& mAPVTS;

public:
	explicit SchroederReverbEditor(juce::AudioProcessorValueTreeState& apvts);
	~SchroederReverbEditor() override = default;
	void resized() override;

};

class SchroederReverbProcessor{
private:
    float currentSampleRate { defaultSampleRate };


    bool isOpen { false };
    float decayLevel { 0.5f };//反馈梳状滤波器反馈度
    float diffusionLevel { 0.5f };//扩散全通滤波器的扩散度
    float mixLevel { 0.5f };//干湿混合度
    float dampHz { 2000.0f };//低通滤波器的截止频率
    float roomSize { 0.5f };//房间尺寸，影响延迟时间
    float baseDelayTimeMs { 50.0f };//基础延迟时间，
    float makeUpGainDB { 0.0f };//补偿增益

    std::vector<float> dryTable;//干信号增益查找表，避免每次处理都进行powf计算
    std::vector<float> wetTable;//湿信号增益查找表，避免每次处理都进行powf计算

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDecayLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDiffusionLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMixLevel { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedDampHz { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedRoomSize { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedBaseDelayTimeMs { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
        mSmoothedMakeUpGainDB { 1.0f };

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int startSample,
        int numSamples,
        int numChannels);

    juce::AudioProcessorValueTreeState& mAPVTS;

    

public:
    explicit SchroederReverbProcessor(juce::AudioProcessorValueTreeState& apvts);
    ~SchroederReverbProcessor() = default;
    static void createParameterLayout(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    void updateProcessorParameters();

    void processDelay(
		juce::AudioBuffer<float>& buffer,
		int startSample,
		int numSamples,
		int numChannels);

    void syncParametersFromAPVTS();

    void prepareToPlay(double sampleRate, int maximumBlockSize, int numChannels);
};