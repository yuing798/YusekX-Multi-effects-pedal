#include "UtilsEffect.h"
#include <JuceHeader.h>
#include <string>

DuckerProcessor::DuckerProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts) {

}
DuckerEditor::DuckerEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts) {

}

void DuckerEditor::makeDuckerVisible(){

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

}

void DuckerEditor::resized(){
    duckerOpenCloseButton.setBounds(10, 10, 100, 30);
    duckerModeComboBox.setBounds(120, 10, 100, 30);
}

void DuckerProcessor::updateParameters(int selectedId){
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

void DuckerProcessor::createParameterLayout(
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

void DuckerEditor::bindParameters(std::string DuckerOpenId, std::string DuckerModeId) {
    duckerOpenAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        DuckerOpenId,
        duckerOpenCloseButton);

    duckerModeAttachment = std::make_unique<ComboBoxAttachment>(
        mAPVTS,
        DuckerModeId,
        duckerModeComboBox);
}

void DuckerProcessor::syncParametersFromAPVTS(std::string DuckerOpenId, std::string DuckerModeId, float sampleRate) {

    auto preDuckerMode = duckerMode;

    if (auto* openParameter = mAPVTS.getRawParameterValue(DuckerOpenId))
        isOpen = openParameter->load() >= 0.5f;

    if (auto* modeParameter = mAPVTS.getRawParameterValue(DuckerModeId))
        duckerMode = modeParameter->load() + 1; // 因为ComboBox的ID是从1开始的，所以需要加1

    if(duckerMode != preDuckerMode){
        updateParameters(duckerMode);
        setUpdateValue(sampleRate);
    }
}

float DuckerProcessor::processSample(float drySample, attackAndReleaseFilter& attackAndRelease){


    if(!isOpen)
        return 1.0f;//如果闪避器没有打开，直接返回不衰减的增益

            //转换为dB，1e-6f避免log(0)导致的负无穷大
    float inputSampleDB = juce::Decibels::gainToDecibels(std::abs(drySample) + 1e-6f);
    float gainDB = 0.0f;

    float slope = 1.0f - 1.0f / ratio;
    float over = inputSampleDB - thresoldDB;
    float k = over + kneeRangeDB * 0.5f;//中间系数，方便计算，没有物理意义
    if(over > kneeRangeDB * 0.5f){
        //(targetDB - thresoldDB) / (inputDB - thresoldDB) = 1 / ratio
        gainDB =  - over * slope;
    } else if(over > -kneeRangeDB * 0.5f){
        //在 $over = -W/2$ 处，它的值是 $0$，且斜率也是 $0$（完美衔接不压缩区域）。
        // 在 $over = W/2$ 处，它的值和斜率都与直线压缩区域完全相等。
        gainDB = - slope * k * k / (2 * kneeRangeDB);
    }


    if(gainDB < attackAndRelease.y1){//和前一个采样值比较进行逻辑判断，而不是和阈值进行判断
        attackAndRelease.setValue(attackAndRelease.attackAlpha);
    } else{
        attackAndRelease.setValue(attackAndRelease.releaseAlpha);
    }//这个if语句要放在“增益计算 (Gain Computer)：计算如果不考虑平滑，理论上应该压掉多少 dB”之后

    gainDB = attackAndRelease.processSample(gainDB);

    gainDB = juce::jmax(gainDB, maxAttenuationDB);//限制最大衰减量

    //将处理后的dB值转换回线性增益
    float gain = juce::Decibels::decibelsToGain(gainDB) ;

    return gain;
}
void DuckerProcessor::prepareToPlay(float sampleRate){
    setUpdateValue(sampleRate);
    attackAndReleaseFilters[0].setValue(attackAndReleaseFilters[0].attackAlpha);
    attackAndReleaseFilters[1].setValue(attackAndReleaseFilters[1].attackAlpha);
}

void DuckerProcessor::setUpdateValue(float sampleRate){
        attackAndReleaseFilters[0].setAttackAlpha(sampleRate, attackTimeMS);
        attackAndReleaseFilters[0].setReleaseAlpha(sampleRate, releaseTimeMS);
        attackAndReleaseFilters[1].setAttackAlpha(sampleRate, attackTimeMS);
        attackAndReleaseFilters[1].setReleaseAlpha(sampleRate, releaseTimeMS);
}