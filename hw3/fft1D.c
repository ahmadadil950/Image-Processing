#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793


typedef struct {
        int len;	// length of complex number list
        float * real;	// pointer to real number list
        float * imag;	// pointer to imaginary number list
}

complexS, *complexP;

void fft1D(complexP q1, int dir, complexP q2)
{
    int i, N, N2;

    float * r1, * i1, * r2, * i2, * ra, * ia, * rb, * ib;
    float FCTR, fctr, a, b, c, s, num[2];
    complexP qa, qb;

    N = q1 -> len;
    r1 = (float *) q1 ->real;
    i1 = (float *) q1 ->imag;
    r2 = (float *) q2 ->real;
    i2 = (float *) q2 ->imag;

    if(N == 2){
        a = r1[0] + r1[1];	// F(0)=f(0)+f(1);F(1)=f(0)-f(1)
        b = i1[0] + i1[1];	// a,b needed when r1=r2
        r2[1] = r1[0] - r1[1];
        i2[1] = i1[0] - i1[1];
        r2[0] = a;
        i2[0] = b;
    }
    else
    {
        N2 = N / 2;
        qa = (complexP) malloc(sizeof(complexS));
        qa -> len = N2;
        qa -> real = (float * ) malloc(sizeof(float) * qa -> len);
        qa -> imag = (float * ) malloc(sizeof(float) * qa -> len);

        qb = (complexP) malloc(sizeof(complexS));
        qb -> len = N2;
        qb -> real = (float * ) malloc(sizeof(float) * qb -> len);
        qb -> imag = (float * ) malloc(sizeof(float) * qb -> len);

        ra = (float * ) qa -> real;
        ia = (float * ) qa -> imag;
        rb = (float * ) qb -> real;
        ib = (float * ) qb -> imag;

        // split list into 2 halves; even and odd
        for (i = 0; i < N2; i++) {
            ra[i] = * r1++;
            ia[i] = * i1++;
            rb[i] = * r1++;
            ib[i] = * i1++;
        }

        // compute fft on both lists
                fft1D(qa, dir, qa);
                fft1D(qb, dir, qb);

                // build up coefficients
                if (!dir)	// forward
                    FCTR = -2 * PI / N;
                else	
                    FCTR =  2 * PI / N;

                for (fctr = i = 0; i < N2; i++, fctr += FCTR) {
                    c = cos(fctr);
                    s = sin(fctr);
                    a = c * rb[i] - s * ib[i];
                    r2[i] = ra[i] + a;
                    r2[i + N2] = ra[i] - a;

                    a = s * rb[i] + c * ib[i];
                    i2[i] = ia[i] + a;
                    i2[i + N2] = ia[i] - a;
                }

                free(qa);
                free(qb);
        }

        if (!dir) {	// inverse : divide by logN
                for (i = 0; i < N; i++) {
                    q2 -> real[i] = q2 -> real[i] / 2;
                    q2 -> imag[i] = q2 -> imag[i] / 2;
                }
            }
}

int main(int argc, char* argv[])
{
    if(argc == 4){
        FILE *input, *output;
        char* in = argv[1];         // first argument "in"
        int dir = atoi(argv[2]);    // second argument "dir"
        char* out = argv[3];        // third argument "out"

        
        input = fopen(in, "r");     // Reading input to file
        output = fopen(out, "w");   // Writing ouput to file

        int zeros = 0;

        int w, h;                                     // decalring width and height
        fscanf(input, "%d\t%d", &w, &h);              // reading input file for width and height
        int N = h;

        if(ceil(log2(N) != floor(log2(N)))){        // checking condition for powers of 2
            int upperBase = floor(log2(N)) + 1;     // # of zeros appended
            zeros = pow(2,upperBase) - N;
        }
        if (dir != 0 && dir != 1)
        {
            printf("Invalid Dir command");
            printf("Please use the following commands");
            printf("Forward Fourier Transformation: Dir = 0");
            printf("Inverse Fourier Transformation: Dir = 1");
            return 0;
        }

        complexP q1 = malloc(sizeof(*q1));
        complexP q2 = malloc(sizeof(*q2));

        int space = N + zeros;
        h = space;

        q1->len = space;
        q1->real = malloc(sizeof(float)*space);
        q1->imag = malloc(sizeof(float)*space); 

        q2->len = space;
        q2->real = malloc(sizeof(float)*space);
        q2->imag = malloc(sizeof(float)*space); 
        
        for (int i = 0; i < N; i++)
        {
            fscanf(input, "%f\t%f", &q1->real[i], &q1->imag[i]);
        }

        for (int i = N; i < zeros + N; i++)
        {
            q1->real[i] = q1->imag[i] = 0;
        }

        fft1D(q1,dir,q2);
        fprintf(output, "%d\t%d\n", w, h);
        
        for (int i = 0; i < q2->len; i++)
        {
            fprintf(output, "%f\t%f\n", q2->real[i], q2->imag[i]);
        }
    }

    else
    {
        printf("Invalid arguments");
        printf("Format must be: ./fft1D Input(textfile) Dir(argument) Output(textfile)\n");
    }
    
    return 0;

}

