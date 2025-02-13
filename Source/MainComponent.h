#pragma once

#include <JuceHeader.h>
#include "NoteState.h"
#include "Oscillator.h"

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

    juce::Slider volumeSlider;
    juce::Label  volumeSliderLabel;

    juce::Slider gainSlider;
    juce::Label  gainSliderLabel;

    juce::AudioDeviceManager deviceManager;
    juce::String currentDeviceId;

    Oscillator* oscillator;

    float gain = 1;

    float volume = 0;

    float midiNoteVelocity[NUM_OF_MIDI_NOTES] = {};

    NoteState midiNoteState[NUM_OF_MIDI_NOTES] = {};

    int midiNoteTimer[NUM_OF_MIDI_NOTES] = {};

    // radians
    double midiNoteCurrentAngle[NUM_OF_MIDI_NOTES] = {};

    // radians per sample
    double midiNoteAngleDeltaTable[NUM_OF_MIDI_NOTES] = {};

    bool isPedal = false;

    int pedalOffTime[NUM_OF_MIDI_NOTES] = {};

    float previousVolume[NUM_OF_MIDI_NOTES] = {};

    int attackSamples = 500;

    int holdSamples = 10;

    int decaySamples = 100;

    float sustainVolume = 0.5;
    
    int releaseSamples = 1000;

    int pedalSamples = 120000;

    double sampleRate = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
