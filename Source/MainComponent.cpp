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
    // Gain Slider
    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0, 1);
    gainSlider.onValueChange = [this]
        {
            gain = gainSlider.getValue();
        };
    gainSlider.setValue(gain);

    //==========================================================================
    // Gain Slider label
    addAndMakeVisible(gainSliderLabel);
    gainSliderLabel.setText("Gain", juce::dontSendNotification);

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
            if (midiNoteAmplitude[i] != 0.0f) {
                currentSample += (float)std::sin(midiNoteCurrentAngle[i]);
                midiNoteCurrentAngle[i] += midiNoteAngleDeltaTable[i];
            }
        }
        leftBuffer[sample] = currentSample * gain;
        rightBuffer[sample] = currentSample * gain;
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

    gainSliderLabel.setBounds(labelArea.removeFromTop(40));
    gainSlider.setBounds(area.removeFromTop(40));

}

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    juce::String noteInfo;
    noteInfo << "DeviceName: " << source->getName() << "\n";
    noteInfo << "NoteNumber: " << message.getNoteNumber() << "\n";
    noteInfo << "Velocity:   " << message.getVelocity() << "\n";

    if (message.isSustainPedalOn()) {
        noteInfo << "Sustain Pedal On!" << "\n";
        isSustainHeldDown = true;
    }
    else if (message.isSustainPedalOff()) {
        noteInfo << "Sustain Pedal Off!" << "\n";
        isSustainHeldDown = false;
    }
    else {
        midiNoteAmplitude[message.getNoteNumber()] = (float) message.getVelocity();
    }

    juce::Logger::getCurrentLogger()->writeToLog(noteInfo);
}