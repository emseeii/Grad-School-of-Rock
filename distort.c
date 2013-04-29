/*
 * Written by Conner Hansen, Joseph Blackwell, {add yourself here as you make modifications}
 */

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
#include "extender.h"

const char *PROGRAM_NAME = "distorter";
const char *PROGRAM_VERSION = "0.1";
const char *TYPE_DEFAULT = "default";
const char *TYPE_GRUFF = "gruff";
const char *TYPE_METAL = "metal";
const char *TYPE_SAWTOOTH = "sawtooth";
const char *TYPE_SUBHARMONIC = "subharmonic";
const char *TYPE_NONE = "none";
const int DISTORTER_FOLLOW = 0;
const int DISTORTER_AMPLITUDE = 1;
const int DISTORTER_INVERT_CUTOFF = 2;
const int DISTORTER_SAWTOOTH = 3;
const int DISTORTER_SUBHARMONIC = 4;
const int DISTORTER_NONE = 6;

float cutOffMag = 1.00; // What multiple of the current avg gets clipped
float fadeOut = 0.0; // How long of a fade out should we use
float freqMultiplier = 1.0; // multiplier for subharmonic wave
int segmentsPerSecond = -1; // Number of sample windows to evaluate per second
double segmentsSubdivide = 1.0; // Number of times to divide the segments
int samplesPerFrequencySample = 16000;
int distorterType = 0; // What distorter are we using?
float multiplier = 1.0; // What should the final output be multiplied by?
float secondsToExtend = 0; // How long should the sample be elongated?

static int processOptions(int,char **);
void distortFollow(int* samples, int size, int avg, void (*f)(int,int));
void printArray(int* samples, int size);

////////////////////////////////////////////
// BEGIN DISTORTER BASE IMPLEMENTATIONS
////////////////////////////////////////////

/**
 * Distort along the curve of the falloff of the wave
 */
void distortFollow(int* samples, int size, int avg, void (*f)(int, int)){
	int i;
	
	for(i=0; i<size; ++i){
		
		// Is the magnitude of the sample out of bounds for us?
		if( abs(samples[i]) < (cutOffMag * avg ) )
			printf("%d\n", (int) (multiplier * samples[i]));
		// Yes? Okay, send to whatever our cutoff algorithm is
		else 
			(*f)(samples[i], avg);
	}
}

/**
 * Distorts the entire wave that is generated and not just the values
 * beyond the clipping distance
 * 
 * @param lastValue - the 
 */
int 
distortFull(int* samples, int size, int minIndex, int maxIndex, int lastValue, void (*f)(int, int, int, int)){
	int i;
	
	for(i=0; i<size; ++i){
		(*f)(samples[i], i, minIndex, maxIndex);
	}
	
	return 0;
}

/**
* Distorts the wave into a sawtooth pattern
*/
int distortSawtooth(int* samples, int size)
{
	int place = 0;
	int max = -5000;
	int min = 5000;
	while(place < size)
	{
		//printf("Sample:%d , Max:%d , Min:%d \n", samples[place], max, min);
		if(samples[place] < min)
			min = samples[place];
		
		if(samples[place] > max)
			max = samples[place];
		place++;
	}
	//printf("size:%d , Max:%d , Min:%d \n", size, max, min);
	int slope = (max-min)/size;
	//printf("slope:%d \n", slope);
	place = 0;
	while(place < size)
	{
		samples[place] = (int) (multiplier * ((samples[place] * cutOffMag) + min+slope*place));
		//printf("Sample2:%d , Max:%d , Min:%d \n", samples[place], max, min);
		printf("%d\n", samples[place]);
		place++;
	}
	//int  number;

		//printf("Type in a number \n");
		//scanf("%d", &number);
	
	return 0;
}

/**
 * Distort by adding subharmonic sine waves to the signal
 * if the signal crosses the cutoff line
 * (Probably sounds like crap)
 */
void distortSubHarmonic(int* samples, int size, int avg, void (*f)(int, int))
{
	int i;
	int subharm[size];
	double pi = 3.141592653589793238;
	//initially, there is no subharmonic
	for(i = 0; i < size; i++)
	{
		subharm[i] = 0;
	}
	
	for(i = 0; i < size; i++)
	{
		samples[i] += subharm[i];
		
		// Is the magnitude of the sample out of bounds?
		if(abs(samples[i]) > (cutOffMag * avg))
		{
			int j;
			int sub[size-i];
			int max = 0;
			double amp = 0;
			int freq = 0;
			//find the max amplitude and 1/2 freq of the generated subharmonic
			for(j = i; j < size; j++)
			{
				if(abs(samples[i]) > (cutOffMag * avg))
				{
					if(abs(samples[i]) > max)
					{
						max = abs(samples[i]);
						amp = max - (cutOffMag * avg);
					}
					//this is another sample above the line, so increase the freq by one sample
					freq++;
				}
				else //exit the for loop
					j = size;
			}
			
			//slope of the decay line
			int limit;
			if(size-i < 8000)
				limit = size-i;
			else
				limit = 8000;
			double m = (-amp)/(limit);
			//generate the new sine wave to be added
			for(j = 0; j < limit; j++)
			{
				double n = m*(double)j+amp;
				int newamp = (int)n;
				if(samples[i] > 0)
					sub[j] = -newamp*sin((j*pi)/(freq * freqMultiplier));
				else
					sub[j] = newamp*sin((j*pi)/(freq * freqMultiplier));
			}
			
			//add the new subharmony to the subharm array
			for(j = 0; j < limit; j++)
			{
				subharm[i+j] += sub[j];
			}
		}
		
		//output the sample
		printf("%d\n", (int) (multiplier * samples[i]));
	}
}

////////////////////////////////////////////
// END DISTORTER BASE IMPLEMENTATIONS
////////////////////////////////////////////

////////////////////////////////////////////
// BEGIN CUTOFF IMPLEMENTATIONS
////////////////////////////////////////////

/**
 * Simply cutoff the samples that are over (all are presumed to
 * be over)
 */
void clippingCutoff(int sample, int avg){
	int n = 1;
	
	if( sample < 0 )
		n = -1;
	
	// (+/-1) * (cutOffMagnitude)
	printf("%d\n", (int) (n * (multiplier * cutOffMag * avg) ));
}

/**
 * Invert everything that's going over the cutoff line
 */
void clippingInvertCutoff(int sample, int avg){
	int n = 1;
	int over = abs(sample) - avg;
	
	if( sample < 0 )
		n = -1;
	
	// (+/-1) * (cutOffMagnitude) - howFarOverWasTheSample
	printf("%d\n", (int) (n * ((multiplier * cutOffMag * avg) - over) ));
}

////////////////////////////////////////////
// END CUTOFF IMPLEMENTATIONS
////////////////////////////////////////////

/**
 * Detects the frequency across a specific range of samples
 */
void runFrequencyDetection(int *samples, int start, int stop){
	int i, subSamples[stop - start];
	
	for(i=0; i<stop-start; ++i){
		subSamples[i] = samples[i + start];
	}
	
	// Detects the actual frequency
	detectFrequency(subSamples, stop - start);
}

/**
 * Fade out over a certain number of samples
 */
void fadeByAmount(int *samples, int numSamples, int samplesToFadeOut){
	int i = numSamples - samplesToFadeOut;
	
	if( i < 0 )
		i = 0;
	
	float fadeIncrement = 1.0 / (numSamples - i);
	float currentIncrement = 1.0 - fadeIncrement;
	
	for(i=numSamples - samplesToFadeOut; i<numSamples; ++i){
		samples[i] = (int) samples[i] * currentIncrement;
		currentIncrement -= fadeIncrement;
	}
}

int
main(int argc,char **argv)
    {
	///////////////////////////////////////
	// STANDARD BOILER PLATE SONGLIB CODE
	///////////////////////////////////////
	int argIndex = 1;
	currentFrequencySamples = 0;
	currentFrequency = 0.0;
	verbose = false;

    int i, j, counter, sWindow, avg, numSamples;
    FILE *in,*out;
    RRA *rra;
    
    argIndex = processOptions(argc,argv);

    if (argc-argIndex == 0)
        {
        in = stdin;
        out = stdout;
        }
    else if (argc-argIndex == 1)
        {
        in = fopen(argv[argIndex],"r");
        out = stdout;
        }
    else if (argc-argIndex == 2)
        {
        in = OpenFile(argv[argIndex],"r");
        out = OpenFile(argv[argIndex+1],"w");
        }
    else
        {
        printf("usage: distort [args] [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }
	
	rra = readRRA(in, 0);
	///////////////////////////////////////
	// END STANDARD BOILER PLATE SONGLIB CODE
	///////////////////////////////////////

	// First: load the number of samples we have
	// then load the number of samples we have per second and divide by
	// the number of segments we want per second
	// then make an array of that size for our window
	numSamples = rra->samples;
	sampleRate = rra->sampleRate;
	int *fSamples = malloc( sizeof(int) * samplesPerFrequencySample );
	getSubarray(rra->data[0], &fSamples, 0, samplesPerFrequencySample);
	
	// Find the frequency at the start
	runFrequencyDetection(fSamples, 0, samplesPerFrequencySample);
	
	// Calculate the size of the window
	if( segmentsPerSecond == -1 )
		segmentsPerSecond = (int) (sampleRate / (segmentsSubdivide * currentFrequencySamples));
	
	
	if(distorterType == DISTORTER_SAWTOOTH || distorterType == DISTORTER_SUBHARMONIC)
		sWindow = currentFrequencySamples;
	else
		sWindow = sampleRate / segmentsPerSecond;
	int window[sWindow];
	
	// So, they entered total clip length, which means we need to convert
	// from total clip length to percentage increase...
	// Which would be: clip length that we want / original length of clip
	// ex: if we wanted a clip to be 5 seconds long, and it's originally
	// 1 second long (44,100 samples @ 44,100 samples/s) then we'd have
	// a length multiplier = 5 / 1 = 5;
	float lengthMultiplier = secondsToExtend / (numSamples / sampleRate) ;
	
	// We don't handle shortening
	if( lengthMultiplier < 1.0 )
		lengthMultiplier = 1.0;
	
	// Now, extend the clip by the amount specified
	int *data;
	rra->samples = extendClip(rra->data[0], &data, numSamples, lengthMultiplier);
	numSamples = rra->samples;
	
	int samplesToFadeOut = (int) sampleRate * fadeOut;
	fadeByAmount(data, numSamples, samplesToFadeOut);
	
	// Only print out the header if we're not looking for verbose
	// frequency output
	if( !verbose ) {
		char output[100];
		char index[50];
		strcpy(output, "modifiedBy: distort\n");
		strcat(output, "detectedFrequency: ");
		sprintf(index, "%f", currentFrequency);
		strcat(output, index);
		writeRRAHeader(out,rra,output, 0);
	}
	
	// Go until the end
	for(i=0; i<numSamples; i+=sWindow){
		counter = 0;
		avg = 0;
		
		// Load window
		for(j=0; j<sWindow; ++j){
			if( i + j < numSamples ) {
				window[j] = data[i + j];
				++counter;
				avg += abs(window[j]);
			} else {
				window[j] = 0;
			}
		}
		
		// Find average magnitude
		avg = avg / counter;
		
		if( !verbose ){
			if(distorterType == DISTORTER_FOLLOW)
				distortFollow(window, sWindow, avg, clippingCutoff);
			else if(distorterType == DISTORTER_INVERT_CUTOFF)
				distortFollow(window, sWindow, avg, clippingInvertCutoff);
			else if(distorterType == DISTORTER_SAWTOOTH)
				distortSawtooth(window, sWindow);
			else if(distorterType == DISTORTER_SUBHARMONIC)
				distortSubHarmonic(window, sWindow, avg, clippingCutoff);
			else
				printArray(window, sWindow);
		}
	}
	
	
	fclose(in);
	fclose(out);

	return 0;
}

/**
 * Compares two strings for equality.
 */
static bool
comp(char *left, const char *right){
	int index = 0;
	bool matches = true;
	
	while( left[index] != 0 && right[index] != 0){
		if( left[index] != right[index] )
			matches = false;
		
		++index;
	}
	
	// if either isn't equal to the terminating character
	// then it wasn't finished processing.
	if( left[index] != 0 || right[index] != 0 )
		matches = false;
	
	return matches;
}

/* only -oXXX  or -o XXX options */

static int
processOptions(int argc, char **argv)
    {
    int argIndex;
    int argUsed;
    int separateArg;
    char *arg;

    argIndex = 1;

    while (argIndex < argc && *argv[argIndex] == '-') {

        separateArg = 0;
        argUsed = 0;

        if (argv[argIndex][2] == '\0')
            {
            arg = argv[argIndex+1];
            separateArg = 1;
            }
        else
            arg = argv[argIndex]+2;

        switch (argv[argIndex][1])
            {
			case 'a':{
				multiplier = atof(arg);
				argUsed = 1;
				break;
			}
			case 'c':{
				cutOffMag = atof(arg);
				argUsed = 1;
				break;
			}
			case 'd':{
				distorterType = atoi(arg);
				argUsed = 1;
				break;
			}
			case 'f':{
				fadeOut = atof(arg);
				argUsed = 1;
				break;
			}
			case 'h':{
				printf("usage: distorter [args] [input, [output]]\n");
				printf("-c [N]: specifies the factor at which to begin cutting off the signal. Default is 1.0 (higher gives less cuttoff)\n");
				printf("-d [N]: specifies the distorter type\n");
				printf("\tAvailable types:\n");
				printf("\t0 - Normal distorter that continues to clip even as the amplitude drops off\n");
				printf("\t1 - Flat line distorter that only cuts signals off over a single magnitude\n");
				printf("\t2 - Inverted cutoff distorter that flips the amplitudes beyond the cutoff limit over the cutoff limit\n");
				printf("-f [N]: specifies the number of seconds to fade out over. Default is 0. \n");
				printf("-h - prints this help dialog\n");
				printf("-l [N] - extends the sample to the specified length\n");
				printf("-m [N] - frequency multiplier to be used to increase or decrease the subharmonic frequency used in the subharmonic distorter\n");
				printf("-p - print out frequency information, suppresses amplitude dump\n");
				printf("-t - use a template\n");
				printf("\tdefault - use all default settings, gives a lessened distortion effect\n");
				printf("\tgruff - use rougher settings, more of a metal sound\n");
				printf("\t\n");
				printf("-s [N] - specifies the ratio by which to multiply the sample rate frame size \n");
				
				exit(0);
				break;
			}
			case 'l':{
				secondsToExtend = atof(arg);
				argUsed = 1;
				break;
			}
			case 'm':{
				freqMultiplier = atof(arg);
				argUsed = 1;
				break;
			}
			case 's':{
				segmentsSubdivide = atof(arg);
				
				if(segmentsSubdivide < 0.01)
					segmentsSubdivide = 0.01;
				
				argUsed = 1;
				break;
			}
			case 'p':{
				verbose = true;
				break;
			}
			case 't':{
				int type = -1;
				if( comp( arg, TYPE_DEFAULT )) {
					type = 0;
					distorterType = DISTORTER_FOLLOW;
				} else if( comp( arg, TYPE_GRUFF )) {
					type = 0;
					distorterType = DISTORTER_INVERT_CUTOFF;
					cutOffMag = 0.2;
					multiplier = 2.0;
				} else if( comp(arg, TYPE_SAWTOOTH)) {
					type = 0;
					distorterType = DISTORTER_SAWTOOTH;
				} else if( comp( arg, TYPE_METAL )) {
					type = 0;
					distorterType = DISTORTER_INVERT_CUTOFF;
					cutOffMag = 0.05;
					multiplier = 10.0;
				} else if( comp( arg, TYPE_SUBHARMONIC )) {
					type = 0;
					distorterType = DISTORTER_SUBHARMONIC;
				} else if( comp( arg, TYPE_NONE )) {
					type = 0;
					distorterType = DISTORTER_NONE;
				}
				
				if( type == -1 ) {
					printf("No template found!\n");
					exit(1);
				}
				
				argUsed = 1;
				break;
			}
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            default:
                Fatal("option %s not understood\n", argv[argIndex]);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
    }

