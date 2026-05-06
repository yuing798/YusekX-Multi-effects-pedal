#include "ducker.h"
#include <JuceHeader.h>
#include <string>

void Ducker::makeDuckerVisible(){

    addAndMakeVisible(duckerOpenCloseButton);
    duckerOpenCloseButton.setClickingTogglesState(true);
    duckerOpenCloseButton.onClick = [this] {
        const auto isOpen = duckerOpenCloseButton.getToggleState();
        duckerOpenCloseButton.setButtonText(isOpen ? "close" : "open");
    };

    addAndMakeVisible(duckerModeComboBox);
    duckerModeComboBox.addItem("Light", 1);
    duckerModeComboBox.addItem("Medium", 2);
    duckerModeComboBox.addItem("Heavy", 3);
    duckerModeComboBox.setSelectedId(1);
    duckerModeComboBox.onChange = [this] {
        updateParameters(duckerModeComboBox.getSelectedId());
        //根据selectedId设置闪避器的参数
    };
}

void Ducker::resized(){
    duckerOpenCloseButton.setBounds(10, 10, 100, 30);
    duckerModeComboBox.setBounds(120, 10, 100, 30);
}

void Ducker::updateParameters(int selectedId){
    switch (selectedId) {
        case 1: // Light
            thresoldDB = -30.0f;
            ratio = 2.0f;
            attackTimeMS = 2.0f;
            releaseTimeMS = 80.0f;
            maxAttenuationDB = -10.0f;
            break;
        case 2: // Medium
            thresoldDB = -24.0f;
            ratio = 4.0f;
            attackTimeMS = 2.0f;
            releaseTimeMS = 150.0f;
            maxAttenuationDB = -15.0f;
            break;
        case 3: // Heavy
            thresoldDB = -18.0f;
            ratio = 8.0f;
            attackTimeMS = 2.0f;
            releaseTimeMS = 200.0f;
            maxAttenuationDB = -25.0f;
            break;
        default:
            break;
    }
}

void Ducker::createParameterLayout(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters,
    std::string DuckerOpenId,
    std::string DuckerModeId) {
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { DuckerOpenId, 1 },
        DuckerOpenId,
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { DuckerModeId, 1 },
        DuckerModeId,
        juce::StringArray { "Light", "Medium", "Heavy" },
        0));
}

void Ducker::bindParameters(std::string DuckerOpenId, std::string DuckerModeId) {
    duckerOpenAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        DuckerOpenId,
        duckerOpenCloseButton);

    duckerModeAttachment = std::make_unique<ComboBoxAttachment>(
        mAPVTS,
        DuckerModeId,
        duckerModeComboBox);
}

void Ducker::syncParametersFromAPVTS(std::string DuckerOpenId, std::string DuckerModeId) {
    if (auto* openParameter = mAPVTS.getRawParameterValue(DuckerOpenId))
        isOpen = openParameter->load() >= 0.5f;

    if (auto* modeParameter = mAPVTS.getRawParameterValue(DuckerModeId))
        duckerMode = modeParameter->load() + 1; // 因为ComboBox的ID是从1开始的，所以需要加1

}