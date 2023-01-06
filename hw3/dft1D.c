#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.1415927
#define MAG(i)		(sqrt(Fr[i]*Fr[i] + Fi[i]*Fi[i]))
#define PHASE(i)	(atan2(Fi[i],Fr[i]))

int main(int argc, char *argv[]){
    if(argc == 4){
        FILE *input, *output;
        char* in = argv[1];         // first argument "in"
        int dir = atoi(argv[2]);    // second argument "dir"
        char* out = argv[3];        // third argument "out"

        
        input = fopen(in, "r");     // Reading input to file
        output = fopen(out, "w");   // Writing ouput to file


        int u, x, N;

        double c, s , real, imag, cos(), sin();

        int w, h;                                     // decalring width and height
        fscanf(input, "%d\t%d", &w, &h);              // reading input file for width and height
        N = h;
        /**
         * calculating our fourier coefficents
         * for real and imaginary parts
         */
        float * fr = malloc(sizeof(float) * N);       
        float * Fr = malloc(sizeof(float) * N);
        float * fi = malloc(sizeof(float) * N);
        float * Fi = malloc(sizeof(float) * N);

        fprintf(output, "%d\t%d\n", w, h);          // writing to output file

        // case for F.T DFT
        if(dir == 0){

            for(x=0; x<N; x++) {
                fscanf(input, "%f\t%f", &fr[x], &fi[x]);
            }

            	for(u=0; u<N; u++)  { 
                		                /* compute spectrum over all freq u */
                real = imag = 0;	    /* reset real, imag component of F(u) */
                for(x=0; x<N; x++) {	/* visit each input pixel */

                    c =  cos(2.*PI*u*x/N);
                    s = -sin(2.*PI*u*x/N);
                    real += (fr[x]*c - fi[x]*s);
                    imag += (fr[x]*s + fi[x]*c);

                }
                Fr[u] = real / N;
                Fi[u] = imag / N;
                fprintf(output, "%f\t%f\n", Fr[u], Fi[u]);      // writing to output file
                }
        }
        
       else if(dir == 1) { 								// Inverse DFT
			for (x = 0; x < N; x++) {						// Goes Through Input
				fscanf(input, "%f\t%f", &Fr[x], &Fi[x]);
			}
			for (x = 0; x < N; x++) {		/* compute spectrum over all freq u */
				real = imag = 0;			/* reset real, imag component of F(u) */
				for (u = 0; u < N; u++) {	/* visit each input pixel */
					c = cos(2.*PI*u*x/N);
					s = sin(2.*PI*u*x/N);
					real += (Fr[u]*c - Fi[u]*s);
					imag += (Fr[u]*s + Fi[u]*c);
				}
				fr[x] = real;
				fi[x] = imag;
				fprintf(output, "%f\t%f\n", fr[x], fi[x]); 	// Write to Output
			}
		}
        else                                    // error prompts to user
        {
        printf("Invalid Dir command");
        printf("Please use the following commands");
        printf("Forward Fourier Transformation: Dir = 0");
        printf("Inverse Fourier Transformation: Dir = 1");
        }

        /**
         * freeing memory
         * 
         */
        free(fi);
        free(Fi);
        free(fr);
        free(Fr);
        fclose(input);
        fclose(output);
    }

    else
    {
        printf("Invalid arguments");
        printf("Format must be: ./dtft1D Input(textfile) Dir(argument) Output(textfile)\n");
    }
    
    return 0;
    
}
