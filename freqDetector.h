#ifndef FREQ_DETECTOR
#define FREQ_DETECTOR
int sampleRate; // current sample rate
int currentFrequencySamples; // number of samples per cycle
double currentFrequency; // frequency of the current block
bool verbose;

void printdArray(double *in, int size);
void printArray(int* samples, int size);
int detectFrequency(int *data, int numSamples);
#endif
