

#include "constants.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <memory>
#include <string>
class Ducker : public juce::Component
{
private:
    juce::TextButton duckerOpenCloseButton { "Ducker Open" };
    juce::ComboBox duckerModeComboBox;

    std::unique_ptr<ComboBoxAttachment> duckerModeAttachment;
    std::unique_ptr<ButtonAttachment> duckerOpenAttachment;

    bool isOpen { false };
    int duckerMode { 1 };//默认模式为Light
    float thresoldDB { -24.0f };
    float ratio { 4.0f };
    float attackTimeMS { 2.0f };
    float releaseTimeMS { 150.0f };
    float maxAttenuationDB { -15.0f };//最大衰减量

    juce::AudioProcessorValueTreeState& mAPVTS;

public:

    explicit Ducker(juce::AudioProcessorValueTreeState& apvts);
    ~Ducker() = default;
    
    void resized() override;

    void makeDuckerVisible();

    void updateParameters(int selectedId);

    static void createParameterLayout(
        std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters,
        std::string DuckerOpenId,
        std::string DuckerModeId);

    void bindParameters(std::string DuckerOpenId, std::string DuckerModeId);

    void syncParametersFromAPVTS(std::string DuckerOpenId, std::string DuckerModeId);
};