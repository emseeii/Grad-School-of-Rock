#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <songlib/util.h>
#include <songlib/rra.h>

const char *PROGRAM_NAME = "multiReverb";
const char *PROGRAM_VERSION = "0.12";
const int DISTORTER_FOLLOW = 0;
const int DISTORTER_AMPLITUDE = 1;

float cutOffMag = 1.75;
int segmentsPerSecond = 20;
int distorterType = 0;

static int processOptions(int,char **);

void distortFollow(int* samples, int size, int avg){
	int i;
	
	for(i=0; i<size; ++i){
		if( abs(samples[i]) < (cutOffMag * avg ) )
			printf("%d\n", samples[i]);
		else {
			if( samples[i] < 0 )
				printf("%d\n", -1 * (int) (cutOffMag * avg));
			else 
				printf("%d\n", (int) (cutOffMag * avg));
		}
	}
}

int
main(int argc,char **argv)
    {
    int argIndex = 1;

    int i, j, counter, sWindow, avg;
    FILE *in,*out;
    RRA *rra, *h;
    int numSamples = 0;
    
    

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

	//h = newRRAHeader();
	//readRRAHeader(in,h,0);
	rra = readRRA(in, 0);
	writeRRAHeader(out,rra,"modifiedBy: distort",0);
	numSamples = rra->samples;
	sWindow = numSamples / segmentsPerSecond;
	int window[sWindow];
	
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
		
		// Now print, but clip everything outside of 120% of the average
		if(distorterType == DISTORTER_FOLLOW)
			distortFollow(window, sWindow, avg);
	}
	
	//// I like doing the initial check. Don't just me.
	//if( walls != NULL ){
		//linkedLink *l = walls;
		
		//while( l != NULL ){
			//// Set the delay in samples
			////l->delay = 0.5;
			//l->sdelay = l->delay * rra->sampleRate;
			////l->attenuation = 0.2;
			
			//link *head = (link*) malloc(sizeof(link));
			//link *tail = head;
			
			//head->amplitude = 0;
			
			//for(i = 0; i<l->sdelay; ++i){
				//link *newTail = (link*) malloc(sizeof(link));
				//newTail->amplitude = 0;
				
				//tail->next = newTail;
				//tail = newTail;
			//}
			//// Complete the ring
			//tail->next = head;
			//l->currLink = head;
			
			//// Check the sdelay
			////printf("l->sdelay: %d\n", l->sdelay);
			
			//if( l->sdelay > maxCycle )
				//maxCycle = l->sdelay;
				
			//// Get the next set of links
			//l = l->nextLink;
		//}
	//}
	
	//int samples = rra->samples;
	//int loop = TRUE;
	//int maxValue = 0;
	//link *rraOutput = (link*) malloc(sizeof(link));
	//link *tail = rraOutput;
	//i = 0;
	
	//rraOutput->amplitude = 0;
	//counter = 0;
	//while(loop == TRUE){
		//int adjS = 0;
		//if( i < samples )
			//adjS = rra->data[0][i];
		////int s = rra->data[0][i];
		////int adjS = s;
		//linkedLink *l = walls;
		
		//// Cycle, adjust the value
		//while(l != NULL){
			//adjS += l->currLink->amplitude * l->attenuation;
			//l = l->nextLink;
			//counter++;
		//}
		//if( i < samples )
			//adjS  = adjS / ((int) log10( counter ))+ rra->data[0][i];
		//else
			//adjS  = adjS / counter;
		
		//// Cycle again, now plug the final value back into every list
		//l = walls;
		//while(l != NULL){
			//l->currLink->amplitude = adjS;
			//l->currLink = l->currLink->next;
			//l = l->nextLink;
		//}
		//if( adjS > maxValue )
			//maxValue = adjS;
				
		////~ fprintf(out, "%d\n", adjS);
		//link *newTail = (link*) malloc(sizeof(link));
		//newTail->amplitude = adjS;
		
		//tail->next = newTail;
		//tail = newTail;
		
		//if( i > samples ){
			//numSamples++;
			//if( adjS > maxValue )
				//maxValue = adjS;
			
			//if( (i - samples)%maxCycle == 0 ){
				//if( maxValue < 10 )
					//loop = FALSE;
				
				//maxValue = 0;
			//}
		//}
		
		//++i;
	//}
	
	
	//link *l = rraOutput;
	//loop = TRUE;
	
	//rra->samples = numSamples;
	//writeRRAHeader(out,rra,"modifiedBy: multiReverb",0);
	
	//while(loop){
		//fprintf(out, "%d\n", l->amplitude);
		
		//l = l->next;
		//if(l == NULL)
			//loop = FALSE;
	//}
	
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
            //case 'd':{
				//// Distance to the wall
				//int value = atoi(arg);
				
				//// Create a new linkedLink
				//linkedLink *newLinks = (linkedLink*) malloc(sizeof(linkedLink));
				
				//// At 100m, the attenuation should be ~ 0.01
				////newLinks->attenuation = 10 / (11 * value);
				//newLinks->attenuation = 1 / pow(1.025, value);
				//// Distance / Speed of Sound = delay
				//newLinks->delay = 2 * value / 340.24;
				
				//if( walls == NULL )
					//walls = newLinks;
				//else {
					//linkedLink *l = walls;
					
					//while( l->nextLink != NULL ){
						//l = l->nextLink;
					//}
					
					//l->nextLink = newLinks;
				//}

				//argUsed = 1;
				//break;
				//}
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
				printf("-c N: specifies the factor at which to begin cutting off the signal. Default is 1.75 (higher gives less cuttoff)\n");
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

