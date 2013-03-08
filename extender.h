#ifndef EXTENDER
#define EXTENDER
double falloffRate; // Number of seconds to fall off at the end

void extendClip(int *data, int **out, int baseSamples, float multiplier);
int findAverage(int* samples, int start, int size);
int findLastValueIndex(int *samples, int value, int size);
void getSubarray(int *data, int **out, int start, int stop);
void normalizeSamples(int *data, int *template, int size);
#endif
