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

int findMaxMagnitude(int **in, int start, int count);

/**
 * Adds the data in add onto the data array, starting approximately
 * at the value of index, and attempts to maintain an overall block
 * average of avg
 * @return the last index that was added on
 */ 
int addOntoSample(int **data, int **add, int index, int avg, int blockSize, int dataSize, int addSize){
	int start = findBestMatch(data, add, index, 1000, blockSize), i, j;
	int stop = start;
	
	for(i=0; i<addSize; i += blockSize){
		int dataMax = findMaxMagnitude(data, start + i, blockSize);
		
		// Find the percent difference
		float multiplier = ((float) avg - dataMax) / avg;
		
		if( multiplier < 0.0 )
			multiplier = 0.0;
		else if( multiplier > 1.0 )
			multiplier = 1.0;
		
		for(j=0; j<blockSize; ++j){
			if( i + j < addSize && start + i + j < dataSize )
				data[0][start + i + j] += (int) (add[0][i + j] * multiplier);
		}
		
		++stop;
	}
	
	return stop;
}

/**
 * Finds the best matching index to begin inserting the wave sample
 */
int findBestMatch(int **arr1, int **arr2, int start, int numChecks, int numSamples){
	int i, j, currStart = start - numChecks/2, bestMatch = 0;
	int avgChecks[numChecks];
	float mse;
	
	for(i=0; i<numChecks; ++i){
		mse = 0.0;
		
		for(j=0; j<numSamples; ++j){
			mse += calcError(arr1[0][currStart+j], arr2[0][j]);
		}
		
		avgChecks[i] = mse / numSamples;
		
		++currStart;
	}
	
	for(i=0; i<numChecks; ++i){
		if(avgChecks[i] < avgChecks[bestMatch])
			bestMatch = i;
	}
	
	return (start - numChecks/2) + bestMatch;
}

/**
 * Finds the minimum value and returns the index for that
 * value
 */
int findMaxMagnitude(int **in, int start, int count){
	int i, maxAmp;
	
	maxAmp = abs(in[0][start]);
	for(i=start; i< start + count; ++i){
		if( abs(in[0][i]) > maxAmp ){
			maxAmp = abs(in[0][i]);
		}
	}
	
	return maxAmp;
}

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
 * Adjusts the samples within each block to match the overall average
 * of the blocks. This only works up until the last occurance of the
 * average value so that the sound will still fall off appropriately.
 */
void normalizeSamplesToAverage(int **data, int **out, int size, int blockSize, int avg){
	int i, j=0, k, max=0;
	*out = malloc( sizeof(int) * size);
	//int avg = findAverageByBlocks(data, 0, size, blockSize) / 2;
	int lastIndex = findLastValueIndex(data, avg, size);
	
	for(i=0; i<size; ++i){
		if( abs(data[0][i]) > max )
			max = abs(data[0][i]);
		
		if(j == blockSize){
			double multiplier = (double) (avg + avg / 20) / max;
			
			for(k = i - blockSize; k <= i; ++k){
				if( lastIndex > k && multiplier <= 1.0)
					out[0][k] = (int) (data[0][k] * multiplier);
				else
					out[0][k] = data[0][k];
			}
			
			j = 0;
			max = 0;
		} else {
			++j;
		}
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
 * Finds the average over a range of samples for each block.
 */
int findAverageByBlocks(int **data, int start, int size, int blockSize){
	int i, avg = 0, count=0;
	
	for(i=start; i<size; ++i){
		avg += abs(data[0][i]);
		++count;
	}
	
	return avg / count;
}

/**
 * Finds the last occurence of a particular value in a set of samples
 */
int findLastValueIndex(int **data, int value, int size){
	int i, returnIndex = -1;
	
	for(i=size-1; i>=0 && returnIndex == -1; --i)
		if( abs(data[0][i]) >= value )
			returnIndex = i;
	
	return returnIndex;
}

/**
 * Finds the nearest zero to the starting index within a given tolerance.
 * The search only moves forward along the song
 */
int findNearestZeroIndex(int **data, int start, int size){
	int i, index = start;
	int sampleTolerance = 5;
	
	for(i=start; i<size; ++i){
		// Check within bounds of the tolerance
		if(data[0][i] < sampleTolerance && data[0][i] > -sampleTolerance){
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
	//*out = malloc( sizeof(int) * size);
	
	for(i=0; i<size; ++i){
		out[0][i] = data[i + start];
	}
}

void getSubarrayFromSamples(int **data, int **out, int start, int stop){
	int i, size = stop - start;
	//*out = malloc( sizeof(int) * size);
	
	for(i=0; i<size; ++i){
		out[0][i] = data[0][i + start];
	}
}

/**
 * Extends the clip by the multiplier amount
 * 
 * @return the new sample length
 */
int extendClip(int *data, int **out, int baseSamples, float multiplier){
	//normalizeSamplesToAverage(data, out, baseSamples, 1000);
	//if( multiplier > 1.0 ){
		//int samples = (int) (baseSamples * multiplier + 0.5); // round up
		//int avgMagnitude = findAverage(data, 0, baseSamples);
		//int index = findLastValueIndex(data, avgMagnitude * 2, baseSamples);
		//*out = malloc( sizeof(int) * samples );
	//} 
	int numSamples = baseSamples;
	int *temp = NULL;
	if( multiplier > 1.0 ){
		numSamples = (int) (baseSamples * multiplier + 0.5); // round up
	}
	
	// Malloc the new array
	*out = malloc( sizeof(int) * numSamples );
	getSubarray(data, out, 0, baseSamples);
	
	if( multiplier > 1.0 ){
		int avg = findAverageByBlocks(out, 0, baseSamples / 2, 250);
		normalizeSamplesToAverage(out, &temp, numSamples, 250, avg);
		
		//getSubarrayFromSamples(&temp, out, 0, numSamples);
		
		//int avg = findAverageByBlocks(out, 0, baseSamples / 2, 1000);
		int currIndex = findLastValueIndex(out, avg, numSamples);
		currIndex = findBestMatch(out, &temp, currIndex, 250, numSamples);
		
		int lastIndex = 0;
		
		while( lastIndex < numSamples ){
			lastIndex = addOntoSample( out, &temp, currIndex, avg, 250, numSamples, baseSamples);
			currIndex = findBestMatch(out, &temp, findLastValueIndex(out, avg, numSamples), 250, numSamples);
		}
		
		//if( currIndex < numSamples ){
			//addOntoSample( out, &temp, currIndex, avg, 250, numSamples, baseSamples);
			//currIndex = findBestMatch(out, &temp, findLastValueIndex(out, avg, numSamples), 250, numSamples);
			
			//addOntoSample( out, &temp, currIndex, avg, 250, numSamples, baseSamples);
			//currIndex = findBestMatch(out, &temp, findLastValueIndex(out, avg, numSamples), 250, numSamples);
			
			////currIndex = addOntoSample( out, &temp, currIndex, avg, 1000, numSamples, baseSamples);
			////currIndex = findBestMatch(out, &temp, findLastValueIndex(out, avg, numSamples), 250, numSamples);
			
			////currIndex = addOntoSample( out, &temp, currIndex, avg, 1000, numSamples, baseSamples);
			////currIndex = findBestMatch(out, &temp, findLastValueIndex(out, avg, numSamples), 250, numSamples);
			
			////currIndex = addOntoSample( out, &temp, currIndex, avg, 1000, numSamples, baseSamples);
			////currIndex = findBestMatch(out, &temp, findLastValueIndex(out, avg, numSamples), 250, numSamples);
			
			////currIndex = addOntoSample( out, &temp, currIndex, avg, 1000, numSamples, baseSamples);
			////currIndex = findBestMatch(out, &temp, findLastValueIndex(out, avg, numSamples), 250, numSamples);
		//}
		
		//while( currIndex < numSamples ){
			//int lastIndex = addOntoSample( out, temp,
			//addOntoSample( 
		//}
	}
	
	/*
	 * Algorithm:
	 * 1: calculate output size, malloc
	 * 2: copy original array to output
	 * 3: Find last avg
	 * 4: Add samples back in at a rate of:
	 * 5:	getCurrentBlockValues - block is ~2-4 frequency length samples
	 * 		if currentBlockValues < avg
	 * 			getSamplesBlockValues - block is same size
	 * 			difference = currentBlockValues - avg
	 * 			mult = difference / sampleBlockValues
	 * 			overlay over output
	 */
	
	return numSamples;
}
