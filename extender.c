#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <songlib/util.h>
#include <songlib/rra.h>
#include "extender.h"
#include "freqDetector.h"

/**
 * Adjusts the samples in the data array to match the overall
 * average magnitude of the template
 */
void normalizeSamples(int *data, int *template, int size){
	int i;
	int dAvg = findAverage(data, 0, size);
	int tAvg = findAverage(template, 0, size);
	// Determine the ratio between the template and the data set
	double multiplier = (double) tAvg / dAvg;
	
	for(i=0; i<size; ++i){
		data[i] = data[i] * multiplier;
	}
}

/**
 * Finds the average over a range of samples
 */
int findAverage(int *data, int start, int size){
	int i, avg = 0;
	
	for(i=start; i<size; ++i){
		avg += abs(data[i]);
	}
	
	return avg / size;
}

/**
 * Finds the last occurence of a particular value in a set of samples
 */
int findLastValueIndex(int *data, int value, int size){
	int i, returnIndex = -1;
	
	for(i=size-1; i>=0 && returnIndex == -1; --i)
		if( abs(data[i]) >= value )
			returnIndex = i;
	
	return returnIndex;
}

/**
 * Finds the nearest zero to the starting index within a given tolerance.
 * The search only moves forward along the song
 */
int findNearestZeroIndex(int *data, int start, int size){
	int i, index = start;
	int sampleTolerance = 5;
	
	for(i=start; i<size; ++i){
		// Check within bounds of the tolerance
		if(data[i] < sampleTolerance && data[i] > -sampleTolerance){
			index = i;
			break;
		}
	}
	
	return index;
}

/**
 * Returns the sub array between start and stop
 */ 
void getSubarray(int *data, int **out, int start, int stop){
	int i, size = stop - start;
	*out = malloc( sizeof(int) * size);
	
	for(i=0; i<size; ++i){
		out[0][i] = data[i + start];
	}
	
	//out = sub;
}

/**
 * Extends the clip by the multiplier amount
 */
void extendClip(int *data, int **out, int baseSamples, float multiplier){
	if( multiplier > 1.0 ){
		int samples = (int) (baseSamples * multiplier + 0.5); // round up
		int avgMagnitude = findAverage(data, 0, baseSamples);
		int index = findLastValueIndex(data, avgMagnitude * 2, baseSamples);
		*out = malloc( sizeof(int) * samples );
		
		
	} 
	
	getSubarray(data, out, 0, baseSamples);
}
