#pragma once

#include <JuceHeader.h>

// range of note number is 0 ~ 127
#define NUM_OF_MIDI_NOTES 128

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, 
                       private juce::MidiInputCallback
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

    //==============================================================================
    // Your private member variables go here...

    juce::Slider gainSlider;
    juce::Label  gainSliderLabel;

    juce::AudioDeviceManager deviceManager;
    juce::String currentDeviceId;

    // 0.0 ~ 1.0
    float gain = 0.0f;

    // midiNoteAmplitude[noteNumber] returns amplitude of note
    float midiNoteAmplitude[NUM_OF_MIDI_NOTES] = {};

    // radians
    double midiNoteCurrentAngle[NUM_OF_MIDI_NOTES] = {};

    // radians per sample
    double midiNoteAngleDeltaTable[NUM_OF_MIDI_NOTES] = {};

    // returns true during the sustain pedal is held down
    bool isSustainHeldDown = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
