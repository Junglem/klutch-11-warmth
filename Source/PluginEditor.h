#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class KLUTCH11WARMTHAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit KLUTCH11WARMTHAudioProcessorEditor(KLUTCH11WARMTHAudioProcessor&);
    ~KLUTCH11WARMTHAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class WarmthLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    };

    KLUTCH11WARMTHAudioProcessor& audioProcessor;
    WarmthLookAndFeel lookAndFeel;

    std::array<std::unique_ptr<juce::Slider>, 11> knobs;
    std::array<std::unique_ptr<juce::Label>, 11> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 11> attachments;

    juce::ComboBox qualityBox, presetBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualityAttachment;
    juce::TextButton presetButton { "LOAD PRESET" };
    juce::Label brandLabel, subtitleLabel, qualityLabel, presetLabel;
    juce::Label meterLabel;
    juce::StringArray controlNames { "DRIVE", "TUBE", "TAPE", "BODY", "PRESENCE", "AIR",
                                      "TONE", "DENSITY", "MIX", "INPUT", "OUTPUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KLUTCH11WARMTHAudioProcessorEditor)
};
