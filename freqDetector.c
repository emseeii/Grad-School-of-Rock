#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <songlib/util.h>
#include <songlib/rra.h>
#include "freqDetector.h"

int numSuperSamples = 1;
int numSubSamples = 20;
double tolerance = 1.5;
double errTolerance = 0.12;
bool printFrequency = false;
bool printNote = false;
bool printStepSize = false;

/**
 * Calculate the squared error between the expected and actual
 * values
 */
int calcError(int exp, int act){
	int r = (act - exp) / 100;
	
	return r * r;
}

/**
 * Normalizes the input array into a double array with values
 * between 0.0 and 1.0
 */
void normalize(int *in, double *out, int size){
	int i, max = 0;
	
	// Find the max
	for(i=0; i<size; ++i){
		if( in[i] > max )
			max = in[i];
	}
	
	// Normalize all samples
	for(i=0; i<size; ++i){
		out[i] = (double) in[i] / max;
	}
}

/**
 * Finds the minimum value and returns the index for that
 * value
 */
int findMin(int *in, int size){
	int i, minAmp, minIndex = 0;
	
	minAmp = in[0];
	for(i=0; i<size; ++i){
		if( in[i] < minAmp ){
			minAmp = in[i];
			minIndex = i;
		}
	}
	
	return minIndex;
}

/**
 * Finds the minimum value within a double array and returns
 * the index for that value
 */
int finddMin(double *in, int start, int size){
	int i, minIndex = start;
	double minAmp = 0.0;
	
	minAmp = in[start];
	for(i=start; i<size; ++i){
		if( in[i] < minAmp ){
			minAmp = in[i];
			minIndex = i;
		}
	}
	
	return minIndex;
}

/**
 * Prints out a double array
 */
void printdArray(double *in, int size){
	int i;
	
	for(i=0; i<size; ++i){
		printf("%f\n", in[i]);
	}
}

/**
 * Prints an int array
 */ 
void printArray(int* samples, int size){
	int i;
	
	for(i=0; i<size; ++i){
		printf("%d\n", samples[i]);
	}
}

int
detectFrequency(int *data, int numSamples)
    {
	//int argIndex = 1;

	int i, j, n, bestGuess, frameSize, minFrame, maxFrame, startingSample;
	int minFreq = 15;
	int maxFreq = 4200;
	int tuningNote = 49;
	int tuningFreq = 440;
	double root = pow(2.0, (1/12.0));
	double guessFrequency;
	bool search = true;
	startingSample = numSamples / 2;
	
	minFrame = sampleRate / maxFreq;
	maxFrame = sampleRate / minFreq;
	
	frameSize = maxFrame - minFrame;
	
	int error[ frameSize ];
	double nError[ frameSize ];
	int windowSize = maxFrame / 4;
	
	for(i=0; i < frameSize; ++i){
		int window[windowSize];
		//int compWindow[windowSize];
		int currHz = minFrame + i;
		
		// Load the current window
		for(j=0; j < windowSize; ++j){
			window[j] = data[startingSample + j];
			
			if(window[j] == 0){
				window[j] = 1;
			}
		}
		
		// Algorithm
		// Get sample window
		// Move down by x distance where x is the current frequency
		// being tested, ranging from minFrame to maxFrame
		// for every sample in numSamples
		// move down by x, sum error over window size
		// Begin subsampling
		int err = 0;
		for(n=0; n < numSubSamples; ++n){
			int newStart = startingSample + (n * currHz);
			int tmpError = 0;
			
			for(j=0; j < windowSize; ++j){
				if( newStart + j < numSamples )
					tmpError += calcError(window[j], data[newStart + j]);
			}
			
			err += tmpError;
		}
		
		err = err / numSubSamples;
		error[i] = err;
	}
	
	normalize(error, nError, frameSize);
	bestGuess = -1;
	int currFrameSize = frameSize;
	int start = currFrameSize - minFrame;
	double confidence = 0.0;
	int count = 3;
	
	while( search ){
		int currGuess = finddMin(nError, start, currFrameSize);
		
		if( bestGuess == -1 ) {
			bestGuess = currGuess;
			confidence = nError[bestGuess];
		} else {
			if( currGuess != bestGuess &&
					(nError[currGuess] <= 
					(nError[bestGuess] + tolerance * nError[bestGuess]) ||
					(nError[bestGuess] < errTolerance)) ){
				
				double tmpConfidence = 0.0;
				int adjIndex = currGuess;
				
				for(i=0; i<count && adjIndex < frameSize; i++){
					int minIndex = adjIndex - (2 * (i + 1));
					int maxIndex = adjIndex + (2 * (i + 1));
					
					if( minIndex < 0 )
						adjIndex = 0;
					
					if( maxIndex >= maxFrame )
						maxIndex = maxFrame - 1;
						
					int pickedIndex = finddMin(nError, minIndex, maxIndex);
					
					tmpConfidence += nError[ pickedIndex ];
					adjIndex = currGuess * (i+2) + minFrame * (i+1);
				}
				
				tmpConfidence = tmpConfidence / count;
				
				if( tmpConfidence < confidence + tolerance * confidence || 
						tmpConfidence < errTolerance){
					bestGuess = currGuess;
					confidence = tmpConfidence;
				}
			}
		}
		
		currFrameSize -= minFrame;
		start -= minFrame;
		
		if(start < 0)
			search = false;
	}
	
	// Okay, get the frequency
	guessFrequency = (double) sampleRate / (bestGuess + minFrame);
	currentFrequency = guessFrequency;
	currentFrequencySamples = bestGuess + minFrame;
	
	// Should we print out note information?
	if( verbose ){
		// Start at 
		int closestNote = 1;
		
		// Test the first note
		double bestFreq = tuningFreq * pow( root, (double) (1 - tuningNote) );
		double err = fabs( (double) bestFreq - guessFrequency ) / guessFrequency;
		
		i = 2;
		for(; i < 160; ++i){
			double testFreq = tuningFreq * pow( root, (double) (i - tuningNote) );
			double testErr = fabs( testFreq - guessFrequency ) / guessFrequency;
			
			if( testErr < err ){
				err = testErr;
				bestFreq = testFreq;
				closestNote = i;
			} else {
				break;
			}
		}
		
		int octaveNote = closestNote % 12;
		int octave = closestNote / 12;
		char *noteChar;
		
		if(octaveNote > 4)
			octave++;
			
		switch(octaveNote){
			case 1: noteChar = "A"; break;
			case 2: noteChar = "A#/Bd"; break;
			case 3: noteChar = "B"; break;
			case 4: noteChar = "C"; break;
			case 5: noteChar = "C#/Dd"; break;
			case 6: noteChar = "D"; break;
			case 7: noteChar = "D#/Ed"; break;
			case 8: noteChar = "E"; break;
			case 9: noteChar = "F"; break;
			case 10: noteChar = "F#/Gd"; break;
			case 11: noteChar = "G"; break;
			case 0: noteChar = "G#/Ad"; break;
		}
		
		if( printFrequency == true )
			printf("%f\n", guessFrequency);
		
		if( printNote == true )
			printf("%s%d\n", noteChar, octave);
		
		if( printStepSize == true )
			printf("%f\n", bestFreq - guessFrequency);
		
		if( printFrequency == false &&
				printNote == false &&
				printStepSize == false ){
			printf("The frequency is approximately: %fhz\n", guessFrequency);
			printf("The closest note is: %s%d\n", noteChar, octave);
			printf("With an expected frequency of: %f\n", bestFreq);
			printf("The error is: %f%%\n", err * 100.0);
		}
	}
	
	return 0;
}
