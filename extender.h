#ifndef EXTENDER
#define EXTENDER
double falloffRate; // Number of seconds to fall off at the end

int extendClip(int *data, int **out, int baseSamples, float multiplier);
int findAverage(int* samples, int start, int size);
int findLastValueIndex(int **samples, int value, int size);
void getSubarray(int *data, int **out, int start, int stop);
void normalizeSamples(int *data, int *template, int size);
int findAverageByBlocks(int **data, int start, int size, int blockSize);
void normalizeSamplesToAverage(int **data, int **out, int size, int blockSize, int avg);
int findBestMatch(int **arr1, int **arr2, int start, int numChecks, int numSamples);
int addOntoSample(int **data, int **add, int index, int avg, int blockSize, int dataSize, int addSize);
#endif
