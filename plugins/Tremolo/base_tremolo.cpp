#include "base_tremolo.h"
#include "constants.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "Utils/table.h"

Tremolo::Tremolo(){
    addAndMakeVisible(title);
    title.setText("Tremolo", juce::dontSendNotification);

    addAndMakeVisible(waveFormShape);
    waveFormShape.addItem("Sine", Sine);
    waveFormShape.addItem("Square", Square);
    waveFormShape.addItem("Triangle", Triangle);
    waveFormShape.setSelectedId(Sine);    
    waveFormShape.onChange = [this]{
        auto selectedId = waveFormShape.getSelectedId();
        updateUI(selectedId);
        resized(); // 重新布局组件
    };

    addAndMakeVisible(openCloseButton);
    openCloseButton.onClick = [this]{
        isOpen = !isOpen;
        if(isOpen){
            openCloseButton.setButtonText("Close");
            isOpen = true;
        }else{
            openCloseButton.setButtonText("Open");
            isOpen = false;
        }
    };

    addAndMakeVisible(frequencyLabel);
    frequencyLabel.setText("Frequency", juce::dontSendNotification);

    addAndMakeVisible(depthLabel);
    depthLabel.setText("Depth", juce::dontSendNotification);

    addAndMakeVisible(mixLabel);
    mixLabel.setText("Mix", juce::dontSendNotification);

    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(0.1, 20.0, 0.1);
    frequencySlider.setValue(5.0);
    frequencySlider.onValueChange = [this]{
        const auto freq = static_cast<float>(frequencySlider.getValue());
        smoothedFrequency.setTargetValue(freq);
        
    };

    addAndMakeVisible(depthSlider);
    depthSlider.setRange(0.0, 1.0, 0.01);
    depthSlider.setValue(0.5);
    depthSlider.onValueChange = [this]{
        const auto depth = static_cast<float>(depthSlider.getValue());
        smoothedDepth.setTargetValue(depth);
       
    };

    addAndMakeVisible(mixSlider);
    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setValue(0.5);
    mixSlider.onValueChange = [this]{
        const auto mix = static_cast<float>(mixSlider.getValue());
        smoothedMix.setTargetValue(mix);
    };

    addAndMakeVisible(dutyCycleLabel);
    dutyCycleLabel.setText("Duty Cycle", juce::dontSendNotification);

    addAndMakeVisible(dutyCycleSlider);
    dutyCycleSlider.setRange(0.01, 0.99, 0.01);
    dutyCycleSlider.setValue(0.5);
    dutyCycleSlider.onValueChange = [this]{
        const auto duty = static_cast<float>(dutyCycleSlider.getValue());
        smoothedDutyCycle.setTargetValue(duty);
    };

    addAndMakeVisible(peakPositionLabel);
    peakPositionLabel.setText("Peak Position", juce::dontSendNotification);

    addAndMakeVisible(peakPositionSlider);
    peakPositionSlider.setRange(0.01, 0.99, 0.01);
    peakPositionSlider.setValue(0.5);
    peakPositionSlider.onValueChange = [this]{
        const auto peak = static_cast<float>(peakPositionSlider.getValue());
        peakPosition.setTargetValue(peak);
    };
}

void Tremolo::resized(){


    auto titleRect = juce::Rectangle<int> (
        10,30,100, 50);
    title.setBounds(titleRect);
    
    auto waveFormShapeSelect = juce::Rectangle<int>(
        10, 90, 100, 50);
    waveFormShape.setBounds(waveFormShapeSelect);

    auto openCloseButtonRect = juce::Rectangle<int>(
        10, 150, 100, 50);
    openCloseButton.setBounds(openCloseButtonRect);

    auto frequencyRect = juce::Rectangle<int>(
        120, 10, 250, 30);
    frequencySlider.setBounds(frequencyRect.removeFromRight(160));
    frequencyLabel.setBounds(frequencyRect.removeFromLeft(90));

    auto depthRect = juce::Rectangle<int>(
        120, 50, 250, 30);
    depthSlider.setBounds(depthRect.removeFromRight(160));
    depthLabel.setBounds(depthRect.removeFromLeft(90));

    auto mixRect = juce::Rectangle<int>(
        120, 90, 250, 30);
    mixSlider.setBounds(mixRect.removeFromRight(160));
    mixLabel.setBounds(mixRect.removeFromLeft(90));
    
    auto dutyCycleRect = juce::Rectangle<int>(
        120, 130, 250, 30);
    dutyCycleSlider.setBounds(dutyCycleRect.removeFromRight(160));
    dutyCycleLabel.setBounds(dutyCycleRect.removeFromLeft(90));

    auto peakPositionRect = juce::Rectangle<int>(
        120, 170, 250, 30);
    peakPositionSlider.setBounds(peakPositionRect.removeFromRight(160));
    peakPositionLabel.setBounds(peakPositionRect.removeFromLeft(90));
}

void Tremolo::updateUI(int waveformID){
    //根据选择的波形类型更新 UI 或其他相关设置
    //不同方波私有属性

    switch(waveformID){
        case Sine:
            // 设置为正弦波相关的 UI 状态
            dutyCycleLabel.setVisible(false);
            dutyCycleSlider.setVisible(false);

            peakPositionLabel.setVisible(false);
            peakPositionSlider.setVisible(false);

            break;
        case Square:
            // 设置为方波相关的 UI 状态
            dutyCycleLabel.setVisible(true);
            dutyCycleSlider.setVisible(true);

            peakPositionLabel.setVisible(false);
            peakPositionSlider.setVisible(false);
            break;
            
        case Triangle:
            // 设置为三角波相关的 UI 状态
            peakPositionLabel.setVisible(true);
            peakPositionSlider.setVisible(true);

            dutyCycleLabel.setVisible(false);
            dutyCycleSlider.setVisible(false);
            break;
        default:
            break;
    }

    
}

void Tremolo::prepareToPlay(double sampleRate){

    currentSampleRate = sampleRate;

    smoothedFrequency.reset(currentSampleRate, 0.01);
    smoothedFrequency.setCurrentAndTargetValue(5.0f);

    smoothedDepth.reset(currentSampleRate, 0.01);
    smoothedDepth.setCurrentAndTargetValue(0.5f);

    smoothedMix.reset(currentSampleRate, 0.01);
    smoothedMix.setCurrentAndTargetValue(0.5f);

    smoothedDutyCycle.reset(currentSampleRate, 0.01);
    smoothedDutyCycle.setCurrentAndTargetValue(0.5f);

    depthGain.reset(currentSampleRate, 0.002);
    depthGain.setCurrentAndTargetValue(1.0f);

    peakPosition.reset(currentSampleRate, 0.01);
    peakPosition.setCurrentAndTargetValue(0.5f);

    sineGainTable = SineLookUpTable(bufferSize);
}

void Tremolo::processTremolo(
    juce::AudioBuffer<float>& buffer, 
    int startSample, 
    int numSamples, 
    int numChannels)
{
    if(!isOpen){
        return;
    }

    if(waveFormShape.getSelectedId() == Sine){
        processSineTremolo(buffer, startSample, numSamples, numChannels);
        return;
    }else if(waveFormShape.getSelectedId() == Square){
        processSquareTremolo(buffer, startSample, numSamples, numChannels);
        return;
    }else if(waveFormShape.getSelectedId() == Triangle){
        processTriangleTremolo(buffer, startSample, numSamples, numChannels);
        return;
    }
        
}



void Tremolo::processSineTremolo(
    juce::AudioBuffer<float>& buffer, 
    int startSample, 
    int numSamples, 
    int numChannels)
{

    if(sineGainTable.empty()){
       return;
    }
    for(int sampleIndex = 0;sampleIndex < numSamples;sampleIndex++){
        float currentFrequency = smoothedFrequency.getNextValue();
        float currentDepth = smoothedDepth.getNextValue();
        float currentMix = smoothedMix.getNextValue();
        
        auto sineIndex = static_cast<int>(sineTableIndex);
        float lfoValue = sineGainTable[sineIndex];
        float depthGain = 1.0f - (currentDepth * ((lfoValue + 1.0f) / 2.0f));
        // 将LFO值从[-1,1]映射到[0,1]

        for(int channel = 0; channel < numChannels; channel++){
            auto* channelData = buffer.getWritePointer(channel, startSample);
            float wetSample = channelData[sampleIndex] * depthGain;
            channelData[sampleIndex] = 
                (wetSample * currentMix) + 
                (channelData[sampleIndex] * (1.0f - currentMix));
        }
        sineTableIndex += (currentFrequency / 
            static_cast<float>(currentSampleRate)) * 
            static_cast<float>(bufferSize);
        if(sineTableIndex >= bufferSize){
            sineTableIndex -= bufferSize;
        }
    }

}

void Tremolo::processSquareTremolo(
    juce::AudioBuffer<float>& buffer, 
    int startSample, 
    int numSamples, 
    int numChannels)
{
    for(int sampleIndex = 0;sampleIndex < numSamples;sampleIndex++){
        float currentFrequency = smoothedFrequency.getNextValue();
        float currentDepth = smoothedDepth.getNextValue();
        float currentMix = smoothedMix.getNextValue();
        float currentDutyCycle = smoothedDutyCycle.getNextValue();

        bool highState = (lfoPhase < currentDutyCycle);
        if(highState){
            depthGain.setTargetValue(1.0f);
        }else{
            depthGain.setTargetValue(1.0f - currentDepth);
        }

        for(int channel = 0; channel < numChannels; channel++){
            auto* channelData = buffer.getWritePointer(channel, startSample);
            float wetSample = channelData[sampleIndex] * depthGain.getNextValue();
            channelData[sampleIndex] = 
                (wetSample * currentMix) + 
                (channelData[sampleIndex] * (1.0f - currentMix));
        }
        lfoPhase += (currentFrequency / static_cast<float>(currentSampleRate));
        if(lfoPhase >= 1.0f){
            lfoPhase -= 1.0f;
        }
    }
}

void Tremolo::processTriangleTremolo(
    juce::AudioBuffer<float>& buffer, 
    int startSample, 
    int numSamples, 
    int numChannels)
{
    for(int sampleIndex = 0;sampleIndex < numSamples;sampleIndex++){
        float currentFrequency = smoothedFrequency.getNextValue();
        float currentDepth = smoothedDepth.getNextValue();
        float currentMix = smoothedMix.getNextValue();
        float currentPeakPosition = peakPosition.getNextValue();

        float lfoValue;
        if(lfoPhase < currentPeakPosition){
            lfoValue = 
                (currentDepth / currentPeakPosition) * lfoPhase + 1.0f -currentDepth;
        }else{
            lfoValue = 
                (currentDepth / (currentPeakPosition - 1.0f)) * (lfoPhase - 1) + 1.0f -currentDepth;
        }


        for(int channel = 0; channel < numChannels; channel++){
            auto* channelData = buffer.getWritePointer(channel, startSample);
            float wetSample = channelData[sampleIndex] * lfoValue;
            channelData[sampleIndex] = 
                (wetSample * currentMix) + 
                (channelData[sampleIndex] * (1.0f - currentMix));
        }
        lfoPhase += (currentFrequency / static_cast<float>(currentSampleRate));
        if(lfoPhase >= 1.0f){
            lfoPhase -= 1.0f;
        }
    }
}

