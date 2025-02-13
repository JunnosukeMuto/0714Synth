#pragma once
#include <JuceHeader.h>

class Oscillator
{
public:
	float sinOscillator(float radians);
	float distortionOscillator(float radians);
	float currentOscillator(float radians);
	enum oscillatorNumber {
		sin,
		distortion
	};
	void setCurrentOscillator(oscillatorNumber n);
	void setGain(float g);

private:
	oscillatorNumber num = distortion;
	float gain = 1;
};