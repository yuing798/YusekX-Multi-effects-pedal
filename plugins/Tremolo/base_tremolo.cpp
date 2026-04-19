#include "base_tremolo.h"

#include <cmath>

#include "Utils/constants.h"
#include "Utils/table.h"



BaseTremoloProcessor::BaseTremoloProcessor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
}

BaseTremoloEditor::BaseTremoloEditor(juce::AudioProcessorValueTreeState& apvts)
    : mAPVTS(apvts)
{
    addAndMakeVisible(mTitle);
    mTitle.setText("Tremolo Effect", juce::dontSendNotification);

    addAndMakeVisible(mWaveFormShape);
    mWaveFormShape.addItem("Sine", Sine);
    mWaveFormShape.addItem("Square", Square);
    mWaveFormShape.addItem("Triangle", Triangle);
    mWaveFormShape.onChange = [this]
    {
        updateUI(mWaveFormShape.getSelectedId());
        resized();
    };

    addAndMakeVisible(mOpenCloseButton);
    mOpenCloseButton.setClickingTogglesState(true);
    mOpenCloseButton.onClick = [this]
    {
        const auto isOpen = mOpenCloseButton.getToggleState();
        mOpenCloseButton.setButtonText(isOpen ? "Close" : "Open");
    };

    addAndMakeVisible(mFrequencyLabel);
    mFrequencyLabel.setText("Frequency", juce::dontSendNotification);
    addAndMakeVisible(mFrequencySlider);

    addAndMakeVisible(mDepthLabel);
    mDepthLabel.setText("Depth", juce::dontSendNotification);
    addAndMakeVisible(mDepthSlider);

    addAndMakeVisible(mMixLabel);
    mMixLabel.setText("Mix", juce::dontSendNotification);
    addAndMakeVisible(mMixSlider);

    addAndMakeVisible(mDutyCycleLabel);
    mDutyCycleLabel.setText("Duty Cycle", juce::dontSendNotification);
    addAndMakeVisible(mDutyCycleSlider);

    addAndMakeVisible(mPeakPositionLabel);
    mPeakPositionLabel.setText("Peak Position", juce::dontSendNotification);
    addAndMakeVisible(mPeakPositionSlider);

    bindParameters();
    updateUI(mWaveFormShape.getSelectedId());
    mOpenCloseButton.setButtonText(mOpenCloseButton.getToggleState() ? "Close" : "Open");
}

void BaseTremoloEditor::bindParameters()
{
    mOpenCloseButtonAttachment = std::make_unique<ButtonAttachment>(
        mAPVTS,
        BaseTremoloOpenId,
        mOpenCloseButton);

    mFrequencySliderAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseTremoloFreqId,
        mFrequencySlider);

    mDepthSliderAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseTremoloDepthId,
        mDepthSlider);

    mMixSliderAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseTremoloMixId,
        mMixSlider);

    mDutyCycleSliderAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseTremoloDutyCycleId,
        mDutyCycleSlider);

    mPeakPositionSliderAttachment = std::make_unique<SliderAttachment>(
        mAPVTS,
        BaseTremoloPeakPositionId,
        mPeakPositionSlider);

    mWaveFormShapeAttachment = std::make_unique<ComboBoxAttachment>(
        mAPVTS,
        BaseTremoloWaveFormShapeId,
        mWaveFormShape);
}

void BaseTremoloEditor::updateUI(int waveformID)
{
    const auto showDutyCycle = waveformID == Square;
    const auto showPeakPosition = waveformID == Triangle;

    mDutyCycleLabel.setVisible(showDutyCycle);
    mDutyCycleSlider.setVisible(showDutyCycle);
    mPeakPositionLabel.setVisible(showPeakPosition);
    mPeakPositionSlider.setVisible(showPeakPosition);
}

void BaseTremoloEditor::resized()
{
    mTitle.setBounds(10, 10, 100, 40);
    mWaveFormShape.setBounds(10, 55, 100, 30);
    mOpenCloseButton.setBounds(10, 95, 100, 30);

    mFrequencyLabel.setBounds(130, 10, 90, 30);
    mFrequencySlider.setBounds(225, 10, 165, 30);

    mDepthLabel.setBounds(130, 50, 90, 30);
    mDepthSlider.setBounds(225, 50, 165, 30);

    mMixLabel.setBounds(130, 90, 90, 30);
    mMixSlider.setBounds(225, 90, 165, 30);

    mDutyCycleLabel.setBounds(130, 130, 90, 30);
    mDutyCycleSlider.setBounds(225, 130, 165, 30);

    mPeakPositionLabel.setBounds(130, 170, 90, 30);
    mPeakPositionSlider.setBounds(225, 170, 165, 30);
}

void BaseTremoloProcessor::createParameterLayout(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters)
{
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        BaseTremoloOpenId,
        "Tremolo Open",
        false));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseTremoloFreqId, 1 },
        "Tremolo Frequency",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f),
        5.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseTremoloDepthId, 1 },
        "Tremolo Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseTremoloMixId, 1 },
        "Tremolo Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseTremoloDutyCycleId, 1 },
        "Tremolo Duty Cycle",
        juce::NormalisableRange<float>(0.01f, 0.99f, 0.01f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { BaseTremoloPeakPositionId, 1 },
        "Tremolo Peak Position",
        juce::NormalisableRange<float>(0.01f, 0.99f, 0.01f),
        0.5f));

    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { BaseTremoloWaveFormShapeId, 1 },
        "Tremolo Waveform",
        juce::StringArray { "Sine", "Square", "Triangle" },
        0));
}

void BaseTremoloProcessor::syncParametersFromAPVTS()
{
    if (auto* openParameter = mAPVTS.getRawParameterValue(BaseTremoloOpenId))
        mIsOpen = openParameter->load() >= 0.5f;

    if (auto* frequencyParameter = mAPVTS.getRawParameterValue(BaseTremoloFreqId))
    {
        mFrequency = frequencyParameter->load();

    }

    if (auto* depthParameter = mAPVTS.getRawParameterValue(BaseTremoloDepthId))
    {
        mDepth = depthParameter->load();
      
    }

    if (auto* mixParameter = mAPVTS.getRawParameterValue(BaseTremoloMixId))
        mMix = mixParameter->load();
    if (auto* dutyCycleParameter = mAPVTS.getRawParameterValue(BaseTremoloDutyCycleId))
        mDutyCycle = dutyCycleParameter->load();
    if (auto* peakPositionParameter = mAPVTS.getRawParameterValue(BaseTremoloPeakPositionId))
        mPeakPosition = peakPositionParameter->load();

    if (auto* waveFormParameter = mAPVTS.getRawParameterValue(BaseTremoloWaveFormShapeId))
       mWaveformID = waveFormParameter->load() + 1;
}

void BaseTremoloProcessor::prepareToPlay(double sampleRate)
{
    mCurrentSampleRate = sampleRate;

    mSmoothedFrequency.reset(sampleRate, 0.01);
    mSmoothedDepth.reset(sampleRate, 0.01);
    mSmoothedMix.reset(sampleRate, 0.01);
    mSmoothedDutyCycle.reset(sampleRate, 0.01);
    mSmoothedDepthGain.reset(sampleRate, 0.002);
    mSmoothedPeakPosition.reset(sampleRate, 0.01);

    syncParametersFromAPVTS();
    updateProcessorParameters();
}

void BaseTremoloProcessor::updateProcessorParameters(){
    mSmoothedFrequency.setTargetValue(mFrequency);
    mSmoothedDepth.setTargetValue(mDepth);
    mSmoothedMix.setTargetValue(mMix);
    mSmoothedDutyCycle.setTargetValue(mDutyCycle);
    mSmoothedPeakPosition.setTargetValue(mPeakPosition);

    mSmoothedFrequency.reset(mCurrentSampleRate, 0.01);
    mSmoothedDepth.reset(mCurrentSampleRate, 0.01);
    mSmoothedMix.reset(mCurrentSampleRate, 0.01);
    mSmoothedDutyCycle.reset(mCurrentSampleRate, 0.01);
    mSmoothedPeakPosition.reset(mCurrentSampleRate, 0.01);
}

void BaseTremoloProcessor::processTremolo(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels,
    const std::vector<float>& sineTable)
{
    updateProcessorParameters();
    syncParametersFromAPVTS();

    if (!mIsOpen)
        return;


    switch (mWaveformID)
    {
        case Sine:
            processSineTremolo(buffer, startSample, numSamples, numChannels, sineTable);
            return;

        case Square:
            processSquareTremolo(buffer, startSample, numSamples, numChannels);
            return;

        case Triangle:
            processTriangleTremolo(buffer, startSample, numSamples, numChannels);
            return;


        default:
            return;
    }
}

void BaseTremoloProcessor::processSineTremolo(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels,
    const std::vector<float>& sineTable)
{

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const auto currentFrequency = mSmoothedFrequency.getNextValue();
        const auto currentDepth = mSmoothedDepth.getNextValue();
        const auto currentMix = mSmoothedMix.getNextValue();

        const auto sineIndex = juce::jlimit(
            0,
            static_cast<int>(sineTable.size()) - 1,
            static_cast<int>(mSineTableIndex));

        const auto lfoValue = sineTable[static_cast<size_t>(sineIndex)];
        const auto gain = 1.0f - (currentDepth * ((lfoValue + 1.0f) * 0.5f));

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel, startSample);
            const auto drySample = channelData[sampleIndex];
            const auto wetSample = drySample * gain;
            channelData[sampleIndex] = (wetSample * currentMix) + (drySample * (1.0f - currentMix));
        }

        mSineTableIndex += (currentFrequency / static_cast<float>(mCurrentSampleRate))
            * static_cast<float>(sineTable.size());

        if (mSineTableIndex >= static_cast<float>(sineTable.size()))
            mSineTableIndex -= static_cast<float>(sineTable.size());
    }
}

void BaseTremoloProcessor::processSquareTremolo(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const auto currentFrequency = mSmoothedFrequency.getNextValue();
        const auto currentDepth = mSmoothedDepth.getNextValue();
        const auto currentMix = mSmoothedMix.getNextValue();
        const auto currentDutyCycle = mSmoothedDutyCycle.getNextValue();

        const auto highState = mLfoPhase < currentDutyCycle;
        mSmoothedDepthGain.setTargetValue(highState ? 1.0f : 1.0f - currentDepth);
        const auto currentGain = mSmoothedDepthGain.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel, startSample);
            const auto drySample = channelData[sampleIndex];
            const auto wetSample = drySample * currentGain;
            channelData[sampleIndex] = (wetSample * currentMix) + (drySample * (1.0f - currentMix));
        }

        mLfoPhase += currentFrequency / static_cast<float>(mCurrentSampleRate);
        if (mLfoPhase >= 1.0f)
            mLfoPhase -= 1.0f;
    }
}

void BaseTremoloProcessor::processTriangleTremolo(
    juce::AudioBuffer<float>& buffer,
    int startSample,
    int numSamples,
    int numChannels)
{
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const auto currentFrequency = mSmoothedFrequency.getNextValue();
        const auto currentDepth = mSmoothedDepth.getNextValue();
        const auto currentMix = mSmoothedMix.getNextValue();
        const auto currentPeakPosition = mSmoothedPeakPosition.getNextValue();

        float gain = 1.0f;
        if (mLfoPhase < currentPeakPosition)
            gain = (currentDepth / currentPeakPosition) * mLfoPhase + 1.0f - currentDepth;
        else
            gain = (currentDepth / (currentPeakPosition - 1.0f)) * (mLfoPhase - 1.0f) + 1.0f - currentDepth;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel, startSample);
            const auto drySample = channelData[sampleIndex];
            const auto wetSample = drySample * gain;
            channelData[sampleIndex] = (wetSample * currentMix) + (drySample * (1.0f - currentMix));
        }

        mLfoPhase += currentFrequency / static_cast<float>(mCurrentSampleRate);
        if (mLfoPhase >= 1.0f)
            mLfoPhase -= 1.0f;
    }
}
