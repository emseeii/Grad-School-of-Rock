/*
 * Written by Conner Hansen, {add yourself here as you make modifications}
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <songlib/util.h>
#include <songlib/rra.h>

const char *PROGRAM_NAME = "distorter";
const char *PROGRAM_VERSION = "0.1";
const int DISTORTER_FOLLOW = 0;
const int DISTORTER_AMPLITUDE = 1;

float cutOffMag = 1.00; // What multiple of the current avg gets clipped
int segmentsPerSecond = 20; // Number of sample windows to evaluate per second
int distorterType = 0; // What distorter are we using?
float multiplier =1.0; // What should the final output be multiplied by?

static int processOptions(int,char **);

/**
 * Distort along the curve of the falloff of the wave
 */
void distortFollow(int* samples, int size, int avg){
	int i;
	
	for(i=0; i<size; ++i){
		// Is the magnitude of the sample out of bounds for us?
		if( abs(samples[i]) < (cutOffMag * avg ) )
			printf("%d\n", (int) (multiplier * samples[i]));
		else {
			if( samples[i] < 0 )
				printf("%d\n", -1 * (int) (multiplier * cutOffMag * avg));
			else 
				printf("%d\n", (int) (multiplier * cutOffMag * avg));
		}
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
main(int argc,char **argv)
    {
	///////////////////////////////////////
	// STANDARD BOILER PLATE SONGLIB CODE
    ///////////////////////////////////////
    int argIndex = 1;

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
        printf("usage: distort [<input rra file> [<output rra file>]]\n");
        exit(-1);
        }
	
	rra = readRRA(in, 0);
	writeRRAHeader(out,rra,"modifiedBy: distort",0);
	
	///////////////////////////////////////
	// STANDARD BOILER PLATE SONGLIB CODE
    ///////////////////////////////////////

	// First: load the number of samples we have
	// then load the number of samples we have per second and divide by
	// the number of segments we want per second
	// then make an array of that size for our window
	numSamples = rra->samples;
	sWindow = rra->sampleRate / segmentsPerSecond;
	int window[sWindow];
	
	// Go until the end
	for(i=0; i<numSamples; i+=sWindow){
		counter = 0;
		avg = 0;
		
		// Load window
		for(j=0; j<sWindow; ++j){
			if( i + j < numSamples ) {
				window[j] = rra->data[0][i + j];
				++counter;
				avg += abs(window[j]);
			} else {
				window[j] = 0;
			}
		}
		
		// Find average magnitude
		avg = avg / counter;
		
		// Now print, but clip everything outside of the clipping mag * average
		if(distorterType == DISTORTER_FOLLOW)
			distortFollow(window, sWindow, avg);
		else
			printArray(window, sWindow);
	}
	
	
    fclose(in);
    fclose(out);

    return 0;
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
            /*
             * when option has an argument, do this
             *
             *     examples are -m4096 or -m 4096
             *
             *     case 'm':
             *         MemorySize = atol(arg);
             *         argUsed = 1;
             *         break;
             *
             *
             * when option does not have an argument, do this
             *
             *     example is -a
             *
             *     case 'a':
             *         PrintActions = 1;
             *         break;
             */
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
			case 'h':{
				printf("usage: distorter [args] [input, [output]]\n");
				printf("-c N: specifies the factor at which to begin cutting off the signal. Default is 1.0 (higher gives less cuttoff)\n");
				printf("-d N: specifies the distorter type\n");
				printf("\tAvailable types:\n");
				printf("\t0 - Normal distorter that continues to clip even as the amplitude drops off\n");
				printf("\t1 - Flat line distorter that only cuts signals off over a single magnitude\n");
				printf("-h: prints this help dialog\n");
				printf("-s N: specifies the number of samples to take per second of audio\n");
				break;
			}
			case 's':{
				segmentsPerSecond = atoi(arg);
				argUsed = 1;
				break;
			}
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            default:
                Fatal("option %s not understood\n",argv[argIndex]);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
    }

