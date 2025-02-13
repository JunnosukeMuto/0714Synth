#include "Oscillator.h"

float Oscillator::sinOscillator(float radians) {
	return (float)std::sin(radians);
}

float Oscillator::distortionOscillator(float radians) {
	auto sin = gain * std::sin(radians);
	if (sin > 1) return 1;
	else if (sin < -1) return -1;
	else return sin;
}

float Oscillator::currentOscillator(float radians) {
	switch (num)
	{
	case Oscillator::sin:
		return sinOscillator(radians);
	case Oscillator::distortion:
		return distortionOscillator(radians);
	default:
		break;
	}
}

void Oscillator::setCurrentOscillator(oscillatorNumber n) {
	num = n;
}

void Oscillator::setGain(float g) {
	gain = g;
}