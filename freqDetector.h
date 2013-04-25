#ifndef FREQ_DETECTOR
#define FREQ_DETECTOR
int sampleRate; // current sample rate
int currentFrequencySamples; // number of samples per cycle
double currentFrequency; // frequency of the current block
bool verbose;

void normalize(int *in, double *out, int size);
void printdArray(double *in, int size);
void printArray(int* samples, int size);
int detectFrequency(int *data, int numSamples);
int calcError(int exp, int act);
#endif
