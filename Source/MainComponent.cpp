#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // add callback of all midi devices
    deviceManager.addMidiInputDeviceCallback("", this);

    // enable all midi devices
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (auto input : midiInputs) {
        deviceManager.setMidiInputDeviceEnabled(input.identifier, true);
    }

    //==========================================================================
    // volume Slider
    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0, 1);
    volumeSlider.setSkewFactorFromMidPoint(0.1);
    volumeSlider.onValueChange = [this]
        {
            volume = volumeSlider.getValue();
        };
    volumeSlider.setValue(volume);

    //==========================================================================
    // volume slider label
    addAndMakeVisible(volumeSliderLabel);
    volumeSliderLabel.setText("volume", juce::dontSendNotification);

    //==========================================================================
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    //==========================================================================

    const int A4Frequency = 440;
    const int A4NoteNumber = 69;

    this->sampleRate = sampleRate;

    for (int i = 0; i < NUM_OF_MIDI_NOTES; i++)
    {
        auto frequency = 440 * std::pow(2, (i - A4NoteNumber) / 12.0);
        auto cyclesPerSample = frequency / sampleRate;
        midiNoteAngleDeltaTable[i] = cyclesPerSample * juce::MathConstants<double>::twoPi;
    }
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    //bufferToFill.clearActiveBufferRegion();

    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        float currentSample = 0.0f;

        for (int i = 0; i < NUM_OF_MIDI_NOTES; i++)
        {
            if (midiNoteState[i] == NoteState::On) {

                if (midiNoteTimer[i] < attackSamples) {
                    currentSample += (float)std::sin(midiNoteCurrentAngle[i]) * midiNoteVelocity[i] * (previousVolume[i] + (float)midiNoteTimer[i] / attackSamples * (1 - previousVolume[i]));
                }
                else if (midiNoteTimer[i] < attackSamples + holdSamples) {
                    currentSample += (float)std::sin(midiNoteCurrentAngle[i]) * midiNoteVelocity[i];
                }
                else if (midiNoteTimer[i] < attackSamples + holdSamples + decaySamples) {
                    currentSample += (float)std::sin(midiNoteCurrentAngle[i]) * midiNoteVelocity[i] * (1 + (float)(sustainVolume - 1) / decaySamples * (midiNoteTimer[i] - attackSamples - holdSamples));
                }
                else {
                    currentSample += (float)std::sin(midiNoteCurrentAngle[i]) * midiNoteVelocity[i] * sustainVolume;
                }

                midiNoteCurrentAngle[i] += midiNoteAngleDeltaTable[i];

            }
            else if (midiNoteState[i] == NoteState::Off && midiNoteTimer[i] < releaseSamples) {

                previousVolume[i] = sustainVolume * (1 - (float)midiNoteTimer[i] / releaseSamples);
                currentSample += (float)std::sin(midiNoteCurrentAngle[i]) * midiNoteVelocity[i] * previousVolume[i];
                midiNoteCurrentAngle[i] += midiNoteAngleDeltaTable[i];

            }
            else if (midiNoteState[i] == NoteState::Pedal && midiNoteTimer[i] < pedalSamples) {

                previousVolume[i] = sustainVolume * (1 - sqrtf((float)midiNoteTimer[i] / pedalSamples));
                currentSample += (float)std::sin(midiNoteCurrentAngle[i]) * midiNoteVelocity[i] * previousVolume[i];
                midiNoteCurrentAngle[i] += midiNoteAngleDeltaTable[i];

            }
            else if (midiNoteState[i] == NoteState::PedalOff && midiNoteTimer[i] < releaseSamples && previousVolume[i] != 0) {

                previousVolume[i] = sustainVolume * (1 - sqrtf((float)pedalOffTime[i] / pedalSamples)) * (1 - (float)midiNoteTimer[i] / releaseSamples);
                currentSample += (float)std::sin(midiNoteCurrentAngle[i]) * midiNoteVelocity[i] * previousVolume[i];
                midiNoteCurrentAngle[i] += midiNoteAngleDeltaTable[i];

            }
            else {

                previousVolume[i] = 0;
                midiNoteCurrentAngle[i] = 0;

            }

            midiNoteTimer[i]++;
        }

        //if (currentSample > clippingVolume) {
        //    currentSample = clippingVolume;
        //}
        //else if (currentSample < -clippingVolume) {
        //    currentSample = -clippingVolume;
        //}

        leftBuffer[sample] = currentSample * volume;
        rightBuffer[sample] = currentSample * volume;

    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    auto area = getLocalBounds().reduced(10);
    auto labelArea = area.removeFromLeft(100);

    volumeSliderLabel.setBounds(labelArea.removeFromTop(40));
    volumeSlider.setBounds(area.removeFromTop(40));

}

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{

    juce::String noteInfo;
    noteInfo << "DeviceName: " << source->getName() << "\n";
    noteInfo << "NoteNumber: " << message.getNoteNumber() << "\n";
    noteInfo << "Velocity:   " << message.getVelocity() << "\n";

    if (message.isSustainPedalOn()) {

        noteInfo << "Sustain Pedal On!" << "\n";
        isPedal = true;

    }
    else if (message.isSustainPedalOff()) {

        noteInfo << "Sustain Pedal Off!" << "\n";
        isPedal = false;

        for (int i = 0; i < NUM_OF_MIDI_NOTES; i++) {

            if (midiNoteState[i] == NoteState::Pedal) {

                midiNoteState[i] = NoteState::PedalOff;
                pedalOffTime[i] = midiNoteTimer[i];
                midiNoteTimer[i] = 0;

            }
        }

    }
    else if (message.isNoteOn()) {

        midiNoteState[message.getNoteNumber()] = NoteState::On;
        midiNoteTimer[message.getNoteNumber()] = 0;
        midiNoteVelocity[message.getNoteNumber()] = message.getFloatVelocity();

    }
    else if (message.isNoteOff()) {

        midiNoteState[message.getNoteNumber()] = isPedal ? NoteState::Pedal : NoteState::Off;
        midiNoteTimer[message.getNoteNumber()] = 0;

    }

    juce::Logger::getCurrentLogger()->writeToLog(noteInfo);

}