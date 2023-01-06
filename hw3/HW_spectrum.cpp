#include "IP.h"
#include <stdio.h>
#include <algorithm>
using namespace IP;
using namespace std;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Convolve magnitude and phase spectrum from image I1.
// Output is in Imag and Iphase.
//

// length of a complex number list, pointer that points to the real number list, 
// and pointer that points to an imaginary number list.
struct complexP {
	int len;
	float *real;
	float *imag;
};

// instantiating functions
extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
void paddedImage(ImagePtr I1, ImagePtr I1Padded);
void fft1D(complexP *q1, int dir, complexP *q2);
void fft1DRow(ImagePtr I1, ImagePtr Image1);
void fft1DColumn(ImagePtr I1, ImagePtr Image1, ImagePtr Image2);
void fft1DMagPhase(ImagePtr I1, ImagePtr Image2, float *Magnitude, float *Phase);
float getMin(float arr[], int total);
float getMax(float arr[], int total);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Compute magnitude and phase spectrum from input image I1.
// Save results in Imag and Iphase.
//
void
HW_spectrum(ImagePtr I1, ImagePtr Imag, ImagePtr Iphase)
{
	// dimensions of I1 for the first image width, height, and area.
	int w = I1->width();                            
	int h = I1->height();                           
	int total = w * h;                           
	// compute FFT of the input image
	// PUT YOUR CODE HERE...

	// I2 for padding
	ImagePtr I2; 

	// checking if padding is needed.
	// Where I1 is Pads and I2 is Results.
	// creating a new width, height and total.
	// setting I2 to I1 else
	if (ceil(log2(w)) != floor(log2(w)) || ceil(log2(h)) != floor(log2(h))) { 
		paddedImage(I1, I2);                                              
		w = I2->width();                                    
		h = I2->height();                                            
		total = w * h;                                               
	}
	else {
		I2 = I1;               
	}
	// zero for spectrum and one to output the original image
#if 0
	ChannelPtr<uchar> p1, p2, p3;
	int type;
	IP_copyImageHeader(I2, Imag);
	IP_copyImageHeader(I2, Iphase);
	int newTotal = I2->width() * I2->height();
	for (int ch = 0; IP_getChannel(I2, ch, p1, type); ch++) {
		IP_getChannel(Imag, ch, p2, type);
		IP_getChannel(Iphase, ch, p3, type);
		for (int i = 0; i < newTotal; i++) {
			*p2++ = *p1++;
			*p3++ = 0;
		}
	}
#else
	// getting two different images for row and column.
	// allocating Image1 and Image2, FFT for row and column
	ImagePtr Image1, Image2;    
	Image1->allocImage(w, h, FFT_TYPE);     
	Image2->allocImage(w, h, FFT_TYPE); 
	fft1DRow(I2, Image1);                   
	fft1DColumn(I2, Image1, Image2); 

	// compute magnitute and phase spectrums from FFT image
	// PUT YOUR CODE HERE...
	
	// declaring magnitude, phase, and getting magnitude and phase and changes by reference 
	float *Magnitude = new float[total];
	float *Phase = new float[total];   
	fft1DMagPhase(I2, Image2, Magnitude, Phase);  

	// find min and max of magnitude and phase spectrums
	// PUT YOUR CODE HERE...

	// uses magnitude to get the min and max magnitude
	// uses phase to get the min and max phase
	float minMagnitude = getMin(Magnitude, total); 
	float maxMagnitude = getMax(Magnitude, total);
	float minPhase = getMin(Phase, total);  
	float maxPhase = getMax(Phase, total); 
	// scale magnitude and phase to fit between [0, 255]
	// PUT YOUR CODE HERE...

	// copying image head of I2 to output Imag
	// copying image head of I2 to ouptut Iphase
	IP_copyImageHeader(I2, Imag);        
	IP_copyImageHeader(I2, Iphase);         
	ChannelPtr<uchar> Pmag, Pphase;    
	// normalize max by 256
	maxMagnitude /= 256;                                                               
	int type;
	for (int ch = 0; IP_getChannel(Imag, ch, Pmag, type); ch++) {
		for (int i = 0; i < total; i++) {
			// scales magnitude and clipped
			*Pmag++ = CLIP((Magnitude[i] - minMagnitude) / (maxMagnitude - minMagnitude) * MaxGray, 0, MaxGray); 
		}
	}

	for (int ch = 0; IP_getChannel(Iphase, ch, Pphase, type); ch++) {
		for (int i = 0; i < total; i++) {
			// scales phase and clipped
			*Pphase++ = CLIP((Phase[i] - minPhase) / (maxPhase - minPhase) * MaxGray, 0, MaxGray);   
		}
	}
#endif
}

void paddedImage(ImagePtr I1, ImagePtr I1Padded) {
// 0 for padding around
// 1 for padding right and bottom
#if 0
	// dimensions of I1 for width, and height
	int w = I1->width(); 
	int h = I1->height(); 
	int zerosW = 0;
	int upperBase = floor(log2(w)) + 1;

	// # of 0s to append
	zerosW = pow(2, upperBase) - w;     
	int zerosH = 0;
	upperBase = floor(log2(h)) + 1;
	// # of 0s to append
	zerosH = pow(2, upperBase) - h;       

	// storing value amount needed to pad top and bottom
	// storing value amount needed to pad left and right
	int paddingH = zerosH / 2;
	int paddingW = zerosW / 2;

	// adding max height and max width with zeros
	int newH = h + zerosH; 
	int newW = w + zerosW; 
	// allocating new padded image with a new width and height
	I1Padded->allocImage(newW, newH, BW_TYPE);
	ChannelPtr<uchar> in, out, start;

	int type;
	// initializing the buffer
	for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {  
		IP_getChannel(I1Padded, ch, out, type);    
		// Setting up buffer values starting from the first row and column
		for (int i = 0; i < newH; i++) { 
			for (int j = 0; j < newW; j++) { 
				// seeing if the buffer is outside the image, then sets the buffer values to zero
				// else inside image, sets the buffer values to the same as the image values
				if (i < paddingH || i >= paddingH + h || j < paddingW || j >= paddingW + w) { 
					*out++ = 0; 
				}
				else { 
					*out++ = *in++;
				}
			}
		}
	}
	if (zerosH % 2 != 0) {
		 // adding extra row of zeros
		 for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {  
			  IP_getChannel(I1Padded, ch, out, type);     
			  int lastRow = (w + zerosW)*(h + zerosH - 1);
			  for (int j = 0; j < w + zerosW; j++) {
				   out[lastRow + j] = 0;
			  }
		 }
	}
	if (zerosW % 2 != 0) {
		 // adding extra column of zeros
		 for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {  
			  IP_getChannel(I1Padded, ch, out, type);  
			  int lastColumn = w + zerosW - 1;
			  for (int j = 0; j < w + zerosW; j++) {
				   int index = lastColumn * (j + 1);
				   out[index] = 0;
			  }
		 }
	}
#else
	// dimensions of I1 for width and height
	int w = I1->width();   
	int h = I1->height();    
	int zerosW = 0;
	int upperBase = floor(log2(w)) + 1;
	// # of 0s to append
	zerosW = pow(2, upperBase) - w;
	int zerosH = 0;
	upperBase = floor(log2(h)) + 1;
	// # of 0s to append
	zerosH = pow(2, upperBase) - h; 

	// max height and width adding zeros
	// adding new padded image with a new width and height
	int newH = h + zerosH; 
	int newW = w + zerosW;  
	I1Padded->allocImage(newW, newH, BW_TYPE);
	ChannelPtr<uchar> in, out, start;

	int type;
	// initializing buffer
	for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) { 
		// getting output pointer for channel ch
		IP_getChannel(I1Padded, ch, out, type); 

		// nested for loop that goes through height and width
		for (int i = 0; i < newH; i++) {
			for (int j = 0; j < newW; j++) { 
				// output set  to input, else padded value set to zero
				if (i < h && j < w) {
					*out++ = *in++; 
				}
				else {
					*out++ = 0; 
				}
			}
		}
	}
#endif
}

void fft1D(complexP *q1, int dir, complexP *q2) {
	int i, N, N2;
	float *r1, *i1, *r2, *i2, *ra, *ia, *rb, *ib;
	float FCTR, fctr, a, b, c, s;
	complexP *qa, *qb;

	N = q1->len;
	r1 = q1->real;
	i1 = q1->imag;
	r2 = q2->real;
	i2 = q2->imag;

	if (N == 2) {
		// F(0)=f(0)+f(1)
		// F(1)=f(0)-f(1)
		a = r1[0] + r1[1];	
		b = i1[0] + i1[1];
		r2[1] = r1[0] - r1[1];
		i2[1] = i1[0] - i1[1];
		r2[0] = a;
		i2[0] = b;
	}
	else {
		N2 = N / 2;

		complexP first;
		qa = &first;
		qa->len = N2;
		qa->real = new float[N2];
		qa->imag = new float[N2];

		complexP second;
		qb = &second;
		qb->len = N2;
		qb->real = new float[N2];
		qb->imag = new float[N2];

		ra = qa->real;
		ia = qa->imag;
		rb = qb->real;
		ib = qb->imag;

		// splits the list into 2 halves where each are even and odd
		for (i = 0; i < N2; i++) {
			ra[i] = *r1++;
			ia[i] = *i1++;
			rb[i] = *r1++;
			ib[i] = *i1++;
		}

		// computing FFT
		fft1D(qa, dir, qa);
		fft1D(qb, dir, qb);

		// coefficient build up 
		if (!dir)
			FCTR = -2 * PI / N;
		else
			FCTR = 2 * PI / N;
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
		// freeing up memeory
		delete[] qa->real; 
		delete[] qa->imag; 
		delete[] qb->real;
		delete[] qb->imag; 
	}

	// inverse: divide by logN
	if (!dir) {
		for (i = 0; i < N; i++) {
			q2->real[i] = q2->real[i] / 2;
			q2->imag[i] = q2->imag[i] / 2;
		}
	}
}

void fft1DRow(ImagePtr I1, ImagePtr Image1) {
	// dimensions for I1 for width and height
	int w = I1->width();
	int h = I1->height(); 
	ChannelPtr<float> real, img;
	ChannelPtr<uchar> p1; 
	int type;
	// Image1[0] to real and Image1[1] to Imaginary
	real = Image1[0];  
	img = Image1[1]; 

	complexP c1, c2, *q1, *q2;
	q1 = &c1;
	q2 = &c2;

	// length and pointer set to complex number list, real number list and imaginary number list.
	q1->len = w;                            
	q1->real = new float[w];            
	q1->imag = new float[w];  
	q2->len = w;                    
	q2->real = new float[w];       
	q2->imag = new float[w];     

	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		// FFT Row by Row
		for (int row = 0; row < h; row++) { 
			// FFT Columns
			for (int column = 0; column < q1->len; column++) {
				// storing real values for current row into q1 to real
				// storing imaginary values for current row into q1 to imaginary
				q1->real[column] = *p1++;     
				q1->imag[column] = 0;
			}
			// applying FFT 1D to the current row and output into q2
			fft1D(q1, 0, q2);

			// sets reak output to Image1 and Imaginary output to Image1
			for (int i = 0; i < q2->len; i++) {
				*real++ = q2->real[i];  
				*img++ = q2->imag[i];   
			}
		}
		// freeing up memeory
		delete[] q1->real; 
		delete[] q1->imag; 
		delete[] q2->real;  
		delete[] q2->imag; 
	}
}

void fft1DColumn(ImagePtr I1, ImagePtr Image1, ImagePtr Image2) {
	// dimensions for I1 for width and height
	int w = I1->width();                              // Getting Width
	int h = I1->height();                             // Getting Height
	ChannelPtr<float> real, img, real2, img2;
	// setting Image1[0] to real, Image1[1] to img, Image2[0] Is real2, and Image2[1] to img2
	real = Image1[0]; 
	img = Image1[1];   
	real2 = Image2[0]; 
	img2 = Image2[1];  

	complexP c1, c2, *q1, *q2;
	q1 = &c1;
	q2 = &c2;

	// length and pointer set to complex number list, real number list and imaginary number list.
	q1->len = w;                          
	q1->real = new float[w];       
	q1->imag = new float[w];   
	q2->len = w;            
	q2->real = new float[w];     
	q2->imag = new float[w];           

	for (int column = 0; column < w; column++) {
		// creates a temp for real for future row calc, and for imaginary for future row calc
		ChannelPtr<float> temp_real = real;    
		ChannelPtr<float> temp_img = img;   

		// conducting FFT rows
		// storing real values for current columns into q1 to real.
		// storing imaginary value for current columns into q1 to imag.
		// keeps storing until the very last row of the column and increment temp_real and temp_img
		for (int row = 0; row < h; row++) {     
			q1->real[row] = *temp_real;    
			q1->imag[row] = *temp_img; 
			if (row < h - 1) { 
				temp_real += w; 
				temp_img += w; 
			}
		}

		// applying FFT 1D to the current column and output into q2
		fft1D(q1, 0, q2); 

		// sets reak output to Image2 and Imaginary output to Image2
		for (int i = 0; i < h; i++) {  
			*real2++ = q2->real[i];    
			*img2++ = q2->imag[i];      
		}
		// increment real and img by one
		real++;                            
		img++;                             
	}

	// freeing up memeory
	delete[] q1->real; 
	delete[] q1->imag; 
	delete[] q2->real;
	delete[] q2->imag;
}

void fft1DMagPhase(ImagePtr I1, ImagePtr Image2, float *Magnitude, float *Phase) {
	// dimensions of I1 for the first image width, height, and area.
	int w = I1->width(); 
	int h = I1->height(); 
	int total = w * h;
	ChannelPtr<float> real2, img2;
	
	// setting Image2[0] to real2 and Image2[1] to img2
	real2 = Image2[0];                                                        
	img2 = Image2[1];                                                        

	// looping from zero to total while incrementing i, real2 and img2 by 1 every iteration
	// contains the magnitude and phase forumla
	for (int i = 0; i < total; i++, real2++, img2++) {                     
		Magnitude[i] = sqrt(pow(*real2, 2) + pow(*img2, 2));              
		Phase[i] = atan2(*img2, *real2);                               
	}
}

// assuming the first value is min and looping to compare each value.
// have the min set to the minValue and output the minValue variable.
float getMin(float arr[], int total) {
	float minValue = arr[0];                
	for (int i = 1; i < total; i++)        
		minValue = min(minValue, arr[i]); 
	return minValue;                       
}

// assuming the first value is max and looping to compare each value.
// have the max set to the maxValue and output the maxValue variable.
float getMax(float arr[], int total) {
	float maxValue = arr[0];               
	for (int i = 1; i < total; i++)        
		maxValue = max(maxValue, arr[i]);  
	return maxValue;                     
}