#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    float getParam(juce::AudioProcessorValueTreeState& state, const char* id)
    {
        if (auto* value = state.getRawParameterValue(id))
            return value->load();
        return 0.0f;
    }

    float tubeShape(float x, float amount)
    {
        const float drive = 1.0f + amount * 8.0f;
        const float driven = x * drive;
        const float saturated = std::tanh(driven);
        const float evenHarmonics = driven * driven * (driven >= 0.0f ? 1.0f : -1.0f);
        return juce::jlimit(-1.0f, 1.0f,
            saturated * (1.0f - amount * 0.16f) + evenHarmonics * amount * 0.035f);
    }

    float tapeShape(float x, float amount)
    {
        const float drive = 1.0f + amount * 5.5f;
        const float driven = x * drive;
        const float soft = driven / (1.0f + std::abs(driven) * (0.35f + amount * 1.8f));
        const float rounded = std::tanh(driven * 0.85f);
        return soft * (1.0f - amount * 0.35f) + rounded * amount * 0.35f;
    }

    juce::AudioParameterFloatAttributes knobAttributes(const juce::String& label)
    {
        return juce::AudioParameterFloatAttributes().withLabel(label);
    }
}

KLUTCH11WARMTHAudioProcessor::KLUTCH11WARMTHAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "WARMTH_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
KLUTCH11WARMTHAudioProcessor::createParameterLayout()
{
    using Float = juce::AudioParameterFloat;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<Float>("DRIVE", "DRIVE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.28f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("TUBE", "TUBE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("TAPE", "TAPE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("BODY", "BODY", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("PRESENCE", "PRESENCE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("AIR", "AIR", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("TONE", "TONE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("DENSITY", "DENSITY", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("MIX", "MIX", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.75f, knobAttributes("%")));
    p.push_back(std::make_unique<Float>("INPUT", "INPUT", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f, knobAttributes("dB")));
    p.push_back(std::make_unique<Float>("OUTPUT", "OUTPUT", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f, knobAttributes("dB")));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("QUALITY", "QUALITY",
        juce::StringArray { "2X", "4X", "8X" }, 1));

    return { p.begin(), p.end() };
}

void KLUTCH11WARMTHAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    maximumBlockSize = juce::jmax(1, samplesPerBlock);
    dryBuffer.setSize(getTotalNumOutputChannels(), maximumBlockSize);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    oversampling2x = std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(), 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
    oversampling4x = std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(), 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
    oversampling8x = std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(), 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);

    oversampling2x->init מאximumBlockSize(maximumBlockSize);
    oversampling4x->init מאximumBlockSize(maximumBlockSize);
    oversampling8x->init מאximumBlockSize(maximumBlockSize);
    oversampling2x->prepare(spec);
    oversampling4x->prepare(spec);
    oversampling8x->prepare(spec);

    const auto ramp = 0.025;
    driveSmoother.reset(sampleRate, ramp); tubeSmoother.reset(sampleRate, ramp);
    tapeSmoother.reset(sampleRate, ramp); bodySmoother.reset(sampleRate, ramp);
    presenceSmoother.reset(sampleRate, ramp); airSmoother.reset(sampleRate, ramp);
    toneSmoother.reset(sampleRate, ramp); densitySmoother.reset(sampleRate, ramp);
    mixSmoother.reset(sampleRate, ramp); inputSmoother.reset(sampleRate, ramp);
    outputSmoother.reset(sampleRate, ramp);

    driveSmoother.setCurrentAndTargetValue(getParam(parameters, "DRIVE"));
    tubeSmoother.setCurrentAndTargetValue(getParam(parameters, "TUBE"));
    tapeSmoother.setCurrentAndTargetValue(getParam(parameters, "TAPE"));
    bodySmoother.setCurrentAndTargetValue(getParam(parameters, "BODY"));
    presenceSmoother.setCurrentAndTargetValue(getParam(parameters, "PRESENCE"));
    airSmoother.setCurrentAndTargetValue(getParam(parameters, "AIR"));
    toneSmoother.setCurrentAndTargetValue(getParam(parameters, "TONE"));
    densitySmoother.setCurrentAndTargetValue(getParam(parameters, "DENSITY"));
    mixSmoother.setCurrentAndTargetValue(getParam(parameters, "MIX"));
    inputSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(getParam(parameters, "INPUT")));
    outputSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(getParam(parameters, "OUTPUT")));
}

void KLUTCH11WARMTHAudioProcessor::releaseResources()
{
    oversampling2x.reset();
    oversampling4x.reset();
    oversampling8x.reset();
}

void KLUTCH11WARMTHAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumOutputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < channels; ++ch)
        buffer.clear(ch, 0, samples);

    if (samples > maximumBlockSize || !oversampling2x)
        return;

    const float drive = getParam(parameters, "DRIVE");
    const float tube = getParam(parameters, "TUBE");
    const float tape = getParam(parameters, "TAPE");
    const float body = getParam(parameters, "BODY");
    const float presence = getParam(parameters, "PRESENCE");
    const float air = getParam(parameters, "AIR");
    const float tone = getParam(parameters, "TONE");
    const float density = getParam(parameters, "DENSITY");
    const float mix = getParam(parameters, "MIX");
    const float inGain = juce::Decibels::decibelsToGain(getParam(parameters, "INPUT"));
    const float outGain = juce::Decibels::decibelsToGain(getParam(parameters, "OUTPUT"));

    driveSmoother.setTargetValue(drive); tubeSmoother.setTargetValue(tube);
    tapeSmoother.setTargetValue(tape); bodySmoother.setTargetValue(body);
    presenceSmoother.setTargetValue(presence); airSmoother.setTargetValue(air);
    toneSmoother.setTargetValue(tone); densitySmoother.setTargetValue(density);
    mixSmoother.setTargetValue(mix); inputSmoother.setTargetValue(inGain);
    outputSmoother.setTargetValue(outGain);

    dryBuffer.setSize(channels, samples, false, false, true);
    dryBuffer.makeCopyOf(buffer, true);

    const int quality = juce::roundToInt(getParam(parameters, "QUALITY"));
    auto* os = quality == 0 ? oversampling2x.get()
             : quality == 1 ? oversampling4x.get() : oversampling8x.get();

    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampled = os->processSamplesUp(block);

    for (size_t i = 0; i < oversampled.getNumSamples(); ++i)
    {
        const float driveNow = driveSmoother.getNextValue();
        const float tubeNow = tubeSmoother.getNextValue();
        const float tapeNow = tapeSmoother.getNextValue();
        const float bodyNow = bodySmoother.getNextValue();
        const float presenceNow = presenceSmoother.getNextValue();
        const float airNow = airSmoother.getNextValue();
        const float toneNow = toneSmoother.getNextValue();
        const float densityNow = densitySmoother.getNextValue();
        const float inputNow = inputSmoother.getNextValue();

        const float preGain = inputNow * (1.0f + driveNow * 4.0f);
        const float tubeAmount = juce::jlimit(0.0f, 1.0f, tubeNow * (0.35f + driveNow * 0.65f));
        const float tapeAmount = juce::jlimit(0.0f, 1.0f, tapeNow * (0.35f + driveNow * 0.65f));
        const float stageBlend = juce::jlimit(0.0f, 1.0f, tubeAmount + tapeAmount);

        for (int ch = 0; ch < channels; ++ch)
        {
            const float dry = oversampled.getSample(static_cast<size_t>(ch), i);
            const float driven = dry * preGain;
            const float tubeOut = tubeShape(driven, tubeAmount);
            const float tapeOut = tapeShape(driven, tapeAmount);
            float saturated = driven * (1.0f - stageBlend * 0.5f)
                            + tubeOut * (tubeAmount * 0.5f)
                            + tapeOut * (tapeAmount * 0.5f);

            const float lowWeight = juce::jlimit(0.0f, 1.0f, 0.5f + (bodyNow - 0.5f) * 0.9f);
            const float tilt = (toneNow - 0.5f) * 0.22f;
            const float presenceGain = 1.0f + (presenceNow - 0.5f) * 0.32f;
            const float airGain = 1.0f + airNow * 0.16f;
            saturated *= (0.90f + lowWeight * 0.12f + tilt) * presenceGain * airGain;
            saturated = std::tanh(saturated * (1.0f + densityNow * 0.7f));
            oversampled.setSample(static_cast<size_t>(ch), i, saturated);
        }
    }

    os->processSamplesDown(block);

    const float outputTrim = outGain;
    const float wetMix = mix;
    for (int ch = 0; ch < channels; ++ch)
    {
        auto* wet = buffer.getWritePointer(ch);
        const auto* dry = dryBuffer.getReadPointer(ch);
        for (int i = 0; i < samples; ++i)
        {
            const float mixNow = mixSmoother.getNextValue();
            const float outNow = outputSmoother.getNextValue();
            wet[i] = (dry[i] * (1.0f - mixNow) + wet[i] * mixNow) * outNow;
        }
    }
    juce::ignoreUnused(outputTrim, wetMix);
}

void KLUTCH11WARMTHAudioProcessor::loadFactoryPreset(int index)
{
    static const std::array<std::array<float, 11>, 4> presets {{
        {{0.28f,0.35f,0.25f,0.30f,0.50f,0.25f,0.50f,0.30f,0.75f,0.0f,0.0f}},
        {{0.42f,0.58f,0.12f,0.42f,0.58f,0.30f,0.56f,0.48f,0.72f,-1.0f,-1.0f}},
        {{0.35f,0.18f,0.62f,0.28f,0.48f,0.22f,0.42f,0.56f,0.68f,-1.5f,-0.5f}},
        {{0.18f,0.28f,0.18f,0.20f,0.65f,0.52f,0.62f,0.22f,0.80f,0.0f,-0.5f}}
    }};
    const int selected = juce::jlimit(0, 3, index);
    const char* ids[] = { "DRIVE","TUBE","TAPE","BODY","PRESENCE","AIR","TONE","DENSITY","MIX","INPUT","OUTPUT" };
    for (size_t i = 0; i < 11; ++i)
        if (auto* parameter = parameters.getParameter(ids[i]))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(presets[static_cast<size_t>(selected)][i]));
}

void KLUTCH11WARMTHAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void KLUTCH11WARMTHAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KLUTCH11WARMTHAudioProcessor();
}
