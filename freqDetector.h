#ifndef FREQ_DETECTOR
#define FREQ_DETECTOR
int sampleRate; // current sample rate
int currentFrequencySamples; // number of samples per cycle
double currentFrequency; // frequency of the current block
bool verbose;

int detectFrequency(int *data, int numSamples);
#endif
