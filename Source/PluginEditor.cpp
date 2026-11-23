#include "PluginEditor.h"

namespace
{
    const juce::Colour bg(0xff090909);
    const juce::Colour panel(0xff151311);
    const juce::Colour gold(0xffc79b55);
    const juce::Colour paleGold(0xffe8c98f);
    const juce::Colour muted(0xff8f877c);
}

KLUTCH11WARMTHAudioProcessorEditor::KLUTCH11WARMTHAudioProcessorEditor(KLUTCH11WARMTHAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(900, 650);

    brandLabel.setText("KLUTCH 11", juce::dontSendNotification);
    brandLabel.setFont(juce::Font(juce::FontOptions(29.0f, juce::Font::bold)));
    brandLabel.setColour(juce::Label::textColourId, paleGold);
    addAndMakeVisible(brandLabel);

    subtitleLabel.setText("W A R M T H     /     ANALOG CHARACTER PROCESSOR", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(10.5f)));
    subtitleLabel.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(subtitleLabel);

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        knobs[i] = std::make_unique<juce::Slider>();
        knobs[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[i]->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 20);
        knobs[i]->setColour(juce::Slider::textBoxTextColourId, paleGold);
        knobs[i]->setColour(juce::Slider::textBoxBackgroundColourId, panel);
        knobs[i]->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(*knobs[i]);

        labels[i] = std::make_unique<juce::Label>();
        labels[i]->setText(controlNames[static_cast<int>(i)], juce::dontSendNotification);
        labels[i]->setJustificationType(juce::Justification::centred);
        labels[i]->setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        labels[i]->setColour(juce::Label::textColourId, paleGold);
        addAndMakeVisible(*labels[i]);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, controlNames[static_cast<int>(i)], *knobs[i]);
    }

    qualityBox.addItem("2X OVERSAMPLING", 1);
    qualityBox.addItem("4X OVERSAMPLING", 2);
    qualityBox.addItem("8X OVERSAMPLING", 3);
    qualityBox.setColour(juce::ComboBox::backgroundColourId, panel);
    qualityBox.setColour(juce::ComboBox::textColourId, paleGold);
    qualityBox.setColour(juce::ComboBox::outlineColourId, gold.withAlpha(0.45f));
    qualityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, "QUALITY", qualityBox);
    addAndMakeVisible(qualityBox);

    presetBox.addItem("VOCAL WARMTH", 1);
    presetBox.addItem("TUBE GLOW", 2);
    presetBox.addItem("TAPE THICKNESS", 3);
    presetBox.addItem("AIR & PRESENCE", 4);
    presetBox.setSelectedId(1);
    presetBox.setColour(juce::ComboBox::backgroundColourId, panel);
    presetBox.setColour(juce::ComboBox::textColourId, paleGold);
    presetBox.setColour(juce::ComboBox::outlineColourId, gold.withAlpha(0.45f));
    addAndMakeVisible(presetBox);

    presetButton.setColour(juce::TextButton::buttonColourId, panel);
    presetButton.setColour(juce::TextButton::textColourOffId, paleGold);
    presetButton.setColour(juce::TextButton::buttonOnColourId, gold);
    presetButton.onClick = [this] { audioProcessor.loadFactoryPreset(presetBox.getSelectedId() - 1); };
    addAndMakeVisible(presetButton);

    qualityLabel.setText("PROCESSING QUALITY", juce::dontSendNotification);
    qualityLabel.setColour(juce::Label::textColourId, muted);
    qualityLabel.setFont(juce::Font(juce::FontOptions(9.5f)));
    addAndMakeVisible(qualityLabel);

    presetLabel.setText("FACTORY PRESETS", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, muted);
    presetLabel.setFont(juce::Font(juce::FontOptions(9.5f)));
    addAndMakeVisible(presetLabel);

    meterLabel.setText("STEREO  •  ANALOG-INSPIRED SATURATION  •  VST3", juce::dontSendNotification);
    meterLabel.setColour(juce::Label::textColourId, muted);
    meterLabel.setFont(juce::Font(juce::FontOptions(9.0f)));
    addAndMakeVisible(meterLabel);
}

KLUTCH11WARMTHAudioProcessorEditor::~KLUTCH11WARMTHAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void KLUTCH11WARMTHAudioProcessorEditor::WarmthLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    const float diameter = juce::jmin(width, height) * 0.70f;
    const float radius = diameter * 0.5f;
    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.46f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    g.setColour(juce::Colour(0xff050505));
    g.fillEllipse(cx - radius - 2.0f, cy - radius - 2.0f, diameter + 4.0f, diameter + 4.0f);
    g.setColour(juce::Colour(0xff3b3021));
    g.drawEllipse(cx - radius, cy - radius, diameter, diameter, 2.0f);

    juce::Path arc;
    arc.addCentredArc(cx, cy, radius + 1.0f, radius + 1.0f, 0.0f,
                      startAngle, endAngle, true);
    g.setColour(juce::Colour(0xff40382e));
    g.strokePath(arc, juce::PathStrokeType(3.0f));

    juce::Path valueArc;
    valueArc.addCentredArc(cx, cy, radius + 1.0f, radius + 1.0f, 0.0f,
                           startAngle, angle, true);
    g.setColour(juce::Colour(0xffc79b55));
    g.strokePath(valueArc, juce::PathStrokeType(3.0f));

    g.setColour(juce::Colour(0xff25211c));
    g.fillEllipse(cx - radius + 5.0f, cy - radius + 5.0f,
                  diameter - 10.0f, diameter - 10.0f);

    juce::Path pointer;
    pointer.addRoundedRectangle(-1.8f, -radius + 9.0f, 3.6f, radius * 0.48f, 1.5f);
    g.setColour(juce::Colour(0xffe8c98f));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(cx, cy));
}

void KLUTCH11WARMTHAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(gold.withAlpha(0.65f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(12.0f), 12.0f, 1.0f);

    g.setColour(panel);
    g.fillRoundedRectangle(juce::Rectangle<float>(28.0f, 105.0f, 844.0f, 440.0f), 12.0f);

    g.setColour(gold.withAlpha(0.35f));
    g.drawLine(30.0f, 91.0f, 870.0f, 91.0f, 1.0f);
    g.drawLine(30.0f, 560.0f, 870.0f, 560.0f, 1.0f);

    g.setColour(gold.withAlpha(0.08f));
    for (int i = 0; i < 18; ++i)
        g.drawVerticalLine(42 + i * 48, 118.0f, 532.0f);
}

void KLUTCH11WARMTHAudioProcessorEditor::resized()
{
    brandLabel.setBounds(34, 20, 260, 38);
    subtitleLabel.setBounds(36, 59, 470, 22);

    const int left = 48, top = 125, cellW = 201, cellH = 137;
    for (int i = 0; i < 11; ++i)
    {
        const int row = i / 4;
        const int col = i % 4;
        const int x = left + col * cellW;
        const int y = top + row * cellH;
        labels[static_cast<size_t>(i)]->setBounds(x, y, 170, 20);
        knobs[static_cast<size_t>(i)]->setBounds(x + 12, y + 22, 146, 110);
    }

    qualityLabel.setBounds(54, 552, 160, 18);
    qualityBox.setBounds(54, 574, 190, 30);
    presetLabel.setBounds(280, 552, 150, 18);
    presetBox.setBounds(280, 574, 190, 30);
    presetButton.setBounds(482, 574, 125, 30);
    meterLabel.setBounds(620, 578, 250, 22);
}
