#pragma once

#include <JuceHeader.h>
#include <array>
#include <memory>

class KLUTCH11WARMTHAudioProcessor final : public juce::AudioProcessor
{
public:
    KLUTCH11WARMTHAudioProcessor();
    ~KLUTCH11WARMTHAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;

    void loadFactoryPreset(int index);

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling2x;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling4x;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling8x;

    juce::AudioBuffer<float> dryBuffer;
    double currentSampleRate = 44100.0;
    int maximumBlockSize = 512;

    float smoothDrive = 0.0f;
    float smoothTube = 0.0f;
    float smoothTape = 0.0f;
    float smoothBody = 0.0f;
    float smoothPresence = 0.0f;
    float smoothAir = 0.0f;
    float smoothTone = 0.0f;
    float smoothDensity = 0.0f;
    float smoothMix = 1.0f;
    float smoothInput = 1.0f;
    float smoothOutput = 1.0f;

    juce::SmoothedValue<float> driveSmoother, tubeSmoother, tapeSmoother;
    juce::SmoothedValue<float> bodySmoother, presenceSmoother, airSmoother;
    juce::SmoothedValue<float> toneSmoother, densitySmoother, mixSmoother;
    juce::SmoothedValue<float> inputSmoother, outputSmoother;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KLUTCH11WARMTHAudioProcessor)
};
