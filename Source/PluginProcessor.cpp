/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


auto getPhaserRateName() { return juce::String("Phaser RateHz"); }
auto getPhaserDepthName() { return juce::String("Phaser Depth %"); }
auto getPhaserCenterFreqName() { return juce::String("Phaser Center FreqHz"); } 
auto getPhaserFeedbackName() { return juce::String("Phaser Feedback %"); }
auto getPhaserMixName() { return juce::String("Phaser Mix %"); }

auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay ms"); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %"); }
auto getChorusMixName() { return juce::String("Chorus Mix %"); }

auto getOverdriveSaturationName() { return juce::String("OverDrive Saturation"); }

auto getLadderFilterModeName() { return juce::String("Ladder Filter Mode"); }
auto getLadderFilterCutoffName() { return juce::String("Ladder Filter Cutoff hz"); }
auto getLadderFilterResonanceName() { return juce::String("Ladder Filter Resonance"); }
auto getLadderFilterDriveName() { return juce::String("Ladder Filter Drive"); }

auto getLadderFilterChoices()
{
    return juce::StringArray
    {
        "LPF12",  // low-pass  12 dB/octave
        "HPF12",  // high-pass 12 dB/octave
        "BPF12",  // band-pass 12 dB/octave
        "LPF24",  // low-pass  24 dB/octave
        "HPF24",  // high-pass 24 dB/octave
        "BPF24"   // band-pass 24 dB/octave

    };
    
}

auto getGeneralFilterModeName() { return juce::String("General Filter Mode"); }
auto getGeneralFilterFreqName() { return juce::String("General Filter Freq Hz"); }
auto getGeneralFilterQualityName() { return juce::String("General Filter Quality"); }
auto getGeneralFilterGainName() { return juce::String("General Filter Gain"); }

auto getGeneralFilterChoices() 
{
    return juce::StringArray
    {
        "Peak",
        "bandpass",
        "notch",
        "allpass",

    };

}

//==============================================================================
Project13AudioProcessor::Project13AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    auto floatParams = std::array
    {
        &phaserRateHz,
        &phaserCenterFreqHz,
        &phaserDepthPercent,
        &phaserFeedBackPercent,
        &phaserMixPercent,
            
        &chorusRateHz,
        &chorusCenterDelayMs,
        &chorusDepthPercent,
        &chorusFeedBackPercent,
        &chorusMixPercent,

        &overdriveSaturation,

        //&ladderFilterMode,
        &ladderFilterCutoff,
        &ladderFilterResonance,
        &ladderFilterDrive,

        &generalFilterFreqHz,
        &generalFilterQuality,
        &generalFilterGain,
         
    };

    auto floatNameFuncs = std::array
    {
        &getPhaserRateName,
        &getPhaserDepthName,
        &getPhaserCenterFreqName,
        &getPhaserFeedbackName,
        &getPhaserMixName,

        &getChorusDepthName,
        &getChorusRateName,
        &getChorusCenterDelayName,
        &getChorusFeedbackName,
        &getChorusMixName,

        &getOverdriveSaturationName,

        &getLadderFilterCutoffName,
        &getLadderFilterResonanceName,
        &getLadderFilterDriveName,

        &getGeneralFilterFreqName,
        &getGeneralFilterQualityName,
        &getGeneralFilterGainName,

    };

    for (size_t i = 0; i < floatParams.size(); ++i)
    {
        auto ptrToParamPtr = floatParams[i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(floatNameFuncs[i]()));
        jassert(*ptrToParamPtr != nullptr);

    }


    auto choiceParams = std::array
    {
        &ladderFilterMode,
        &generalFilterMode,

    };

    auto choiceNameFuncs = std::array
    {
        &getLadderFilterModeName,
        &getGeneralFilterModeName,
    };

    for (size_t i = 0; i < choiceParams.size(); ++i)
    {
        auto ptrToParamPtr = choiceParams[i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(choiceNameFuncs[i]()));
        jassert(*ptrToParamPtr != nullptr);

    }




}

Project13AudioProcessor::~ Project13AudioProcessor()
{
}

//==============================================================================
const juce::String Project13AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Project13AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Project13AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Project13AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Project13AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Project13AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Project13AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Project13AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Project13AudioProcessor::getProgramName (int index)
{
    return {};
}

void Project13AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Project13AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();

    std::vector<juce::dsp::ProcessorBase*> dsp
    {
        &phaser,
        &chorus,
        &overdrive,
        &ladderFilter,
        &generalFilter,

    };

    for (auto p : dsp)
    {
        p->prepare(spec);
        p->reset();
    }

}

void Project13AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Project13AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

juce::AudioProcessorValueTreeState::ParameterLayout Project13AudioProcessor::createParameterLayout() 
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const int versionHint = 1;

    /*
        pahser:
            Rate:hz
            Depth:0 to 1
            center freq : hz
            feedback : -1 to +1
            Mix : 0 to 1

    */

    auto name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f, 1.f),
        0.2f,
        "Hz"));


    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.1f, 1.0f, 0.01f, 1.f),
        0.05f,
        "%"));

    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        1000.f,
        "Hz"));

    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
        0.0f,
        "%"));
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.0f, 0.01f, 1.f),
        0.05f,
        "%"));

    /*
        chorus:
            Rate:hz
            Depth:0 to 1
            center delay : ms ( 1 to 100)
            feedback : -1 to +1
            Mix : 0 to 1

    */

    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f),
        0.2f,
        "HZ"));

    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.0f, 0.01f, 1.f),
        0.05f,
        "%"));

    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
        7.f,
        "ms"));

    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
        0.0f,
        "%"));
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.0f, 0.01f, 1.f),
        0.05f,
        "%"));

    /*
        overdrive:
        uses the drive portion of the ladder filter class  for now
        drive :1-100
    */
    //drive : 1 - 100

    name = getOverdriveSaturationName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange < float>(1.f, 100.f, 0.1f, 1.f),
        1.f,
        ""));

    /*
        ladder filter:
            mode:LadderFilterMode enum (int)
            cutoff: hz
            resonance:0 to 1 
            drive :1 - 100
    */

    name = getLadderFilterModeName();
    auto choices = getLadderFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ name, versionHint },
        name,
        choices,
        0));

    name = getLadderFilterCutoffName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 0.1f, 1.f),
        20000.f,
        "Hz"));

    name = getLadderFilterResonanceName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
        0.f,
        ""));

    name = getLadderFilterDriveName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
        1.0f,
        ""));


    /*
    general filter:
    https://docs.juce.com/develop/structdsp_1_1IIR_1_1Coefficients.html
    Mode: Peak, bandpass, notch, allpass,
    freq: 20hz 20,000hz in 1hz steps
    Q: 0.1 10 in 0.05 steps❘ 
    gain: -24db to +24db in 0.5db increments
    */

    name = getGeneralFilterModeName();
    choices = getGeneralFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ name, versionHint },
        name,
        choices,
        0));

    name = getGeneralFilterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        750.f,
        "Hz"));

    name = getGeneralFilterQualityName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f,
        ""));

    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f,
        "dB"));




    return layout;
}

void Project13AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    //[DONE]: add APVTs
    //[DONE]: create audio parameters for all dsp choices
    //[DONE]: prepare all DSP
    //TODO: update audio here from audio parameters
    //TODO: save / load settings
    //TODO: save / load DSP Order
    //TODO: drag-to-reorder GUI 
    //TODO: GUI design for each DSP instance
    //TODO: metering
    //TODO: [Bonuses]

    auto newDSPOrder = DSP_Order();
    
    //try to pull
    while (dspOrderFifo.pull(newDSPOrder)) {

    }

    //if you pulled , replqce dspOrder
    if (newDSPOrder != DSP_Order()) 
        dspOrder = newDSPOrder;

    // now cover to pointers
    DSP_Pointers dspPointers;


    for (size_t i = 0; i < dspPointers.size(); ++i) {

        switch(dspOrder[i])
        {
        case DSP_Option::Phase:
            dspPointers[i] = &phaser;
            break;
        case DSP_Option::Chorus:
            dspPointers[i] = &chorus;
            break;
        case DSP_Option::LadderFilter:
            dspPointers[i] = &ladderFilter;
            break;
        case DSP_Option::Overdrive:
            dspPointers[i] = &overdrive;
            break;
        case DSP_Option::GeneralFilter:
            dspPointers[i] = &generalFilter;
            break;
        case DSP_Option::END_OF_LIST:
            jassertfalse;
            break;
        }
    }

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for(int i =0; i<dspPointers.size() ; ++i) {
        if(dspPointers[i] != nullptr)
        {
            dspPointers[i]->process(context);
        }
    }

    
}

//==============================================================================
bool Project13AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Project13AudioProcessor::createEditor()
{
    return new Project13AudioProcessorEditor (*this);
}

//==============================================================================
void Project13AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Project13AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Project13AudioProcessor();
}
