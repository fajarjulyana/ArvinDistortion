/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ============================================================================== 
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameter.h"

//==============================================================================

DistortionAudioProcessor::DistortionAudioProcessor():
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor (BusesProperties()
                    #if ! JucePlugin_IsMidiEffect
                     #if ! JucePlugin_IsSynth
                      .withInput  ("Input",  AudioChannelSet::stereo(), true)
                     #endif
                      .withOutput ("Output", AudioChannelSet::stereo(), true)
                    #endif
                   ),
#endif
    parameters (*this)
    , paramDistortionType (parameters, "Distortion type", distortionTypeItemsUI, distortionTypeHardClipping)
    , paramInputGain (parameters, "Input gain", "dB", -24.0f, 36.0f, 18.0f,
                      [](float value){ return Decibels::decibelsToGain (value); })
    , paramOutputGain (parameters, "Output gain", "dB", -24.0f, 24.0f, 0.0f,
                       [](float value){ return Decibels::decibelsToGain (value); })
    , paramTone (parameters, "Tone", "dB", -24.0f, 24.0f, 0.0f,
                 [this](float value){ paramTone.setCurrentAndTargetValue (value); updateFilters(); return value; })
{
    parameters.apvts.state = ValueTree (Identifier (getName().removeCharacters ("- ")));
}

DistortionAudioProcessor::~DistortionAudioProcessor()
{
}

//==============================================================================

void DistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const double smoothTime = 1e-3;
    paramDistortionType.reset (sampleRate, smoothTime);
    paramInputGain.reset (sampleRate, smoothTime);
    paramOutputGain.reset (sampleRate, smoothTime);
    paramTone.reset (sampleRate, smoothTime);

    //======================================

    filters.clear();
    for (int i = 0; i < getTotalNumInputChannels(); ++i) {
        Filter* filter;
        filters.add (filter = new Filter());
    }
    updateFilters();
}

void DistortionAudioProcessor::releaseResources()
{
}

void DistortionAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < numInputChannels; ++channel) {
        float* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < numSamples; ++sample) {
            // Ambil sinyal input dan kalikan dengan gain input linier
            const float inputGain = paramInputGain.getNextValue();
            const float in = channelData[sample] * inputGain;
            
            float out = in;

            // Tipe distorsi berdasarkan nilai parameter
            const int distType = (int) paramDistortionType.getTargetValue();

            switch (distType) {
                case distortionTypeHardClipping: {
                        // Multiplier drive internal agar sinyal pasti melewati threshold
                        const float drive = 4.0f;
                        const float drivenIn = in * drive;

                        // Asymmetric Hard Clipping:
                        // Potong batas atas & bawah secara tidak simetris untuk memunculkan even harmonics
                        const float posThreshold = 0.4f;
                        const float negThreshold = -0.7f;

                        if (drivenIn > posThreshold)
                            out = posThreshold;
                        else if (drivenIn < negThreshold)
                            out = negThreshold;
                        else
                            out = drivenIn;

                        // Auto makeup gain / pembatas agar headroom tetap terjaga & tidak terlalu melonjak
                        out *= 0.6f;
                        break;
                    }

                    case distortionTypeSoftClipping: {
                        // Drive internal yang lebih kuat untuk kompresi kurva halus
                        const float drive = 3.5f;
                        const float drivenIn = in * drive;

                        // Soft Clipping menggunakan fungsi Cubic / Polynomial Waveshaper
                        // Memberikan saturasi halus saat sinyal sedang, dan soft clip saat tinggi
                        if (drivenIn > 1.2f) {
                            out = 1.0f;
                        } else if (drivenIn < -1.2f) {
                            out = -1.0f;
                        } else {
                            // Formula cubic waveshaper: x - (x^3 / 3)
                            out = drivenIn - (powf (drivenIn, 3.0f) / 3.0f);
                        }

                        // Kompensasi gain setelah kompresi gelombang
                        out *= 0.7f;
                        break;
                    }
                case distortionTypeExponential: {
                    if (in > 0.0f)
                        out = 1.0f - expf (-in);
                    else
                        out = -1.0f + expf (in);
                    break;
                }
                case distortionTypeFullWaveRectifier: {
                    out = fabsf (in);
                    break;
                }
                case distortionTypeHalfWaveRectifier: {
                    out = (in > 0.0f) ? in : 0.0f;
                    break;
                }
                default:
                    out = in;
                    break;
            }

            // Filter & output gain
            float filtered = filters[channel]->processSingleSampleRaw (out);
            const float outputGain = paramOutputGain.getNextValue();
            channelData[sample] = filtered * outputGain;
        }
    }

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
        buffer.clear (channel, 0, numSamples);
}

//==============================================================================

void DistortionAudioProcessor::updateFilters()
{
    double discreteFrequency = M_PI * 0.01;
    double gain = pow (10.0, (double)paramTone.getTargetValue() * 0.05);

    for (int i = 0; i < filters.size(); ++i)
        filters[i]->updateCoefficients (discreteFrequency, gain);
}

//==============================================================================






//==============================================================================

void DistortionAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = parameters.apvts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.apvts.state.getType()))
            parameters.apvts.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================

AudioProcessorEditor* DistortionAudioProcessor::createEditor()
{
    return new DistortionAudioProcessorEditor (*this);
}

bool DistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================

const String DistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================

int DistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionAudioProcessor::setCurrentProgram (int index)
{
}

const String DistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionAudioProcessor();
}

//==============================================================================
