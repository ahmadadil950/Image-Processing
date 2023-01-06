#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap the phase channels of I1 and I2.
// Output is in II1 and II2.
//

// length of a complex number list, pointer that points to the real number list, 
// and pointer that points to an imaginary number list.
struct complexP {
	int len;      
	float * real;   
	float * imag;   
};

// instantiating external functions
extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
extern void HW_MagPhase2fft(ImagePtr Imag, ImagePtr Iphase, ImagePtr Ifft);
extern void fft1D(complexP *q1, int dir, complexP *q2);
extern void fft1DRow(ImagePtr I1, ImagePtr Image1);
extern void fft1DColumn(ImagePtr I1, ImagePtr Image1, ImagePtr Image2);
extern void fft1DMagPhase(ImagePtr I1, ImagePtr Image2, float *Magnitude, float *Phase);
extern float getMin(float arr[], int total);
extern float getMax(float arr[], int total);

// instantiating swapPhase, Ifft1DRow, Ifft1DColumn functions
void swapPhase(ImagePtr I, float * magnitude, float * newPhase, int total);
void Ifft1DRow(ImagePtr I, ImagePtr IFFT, ImagePtr IFFTout);
void Ifft1DColumn(ImagePtr I, ImagePtr rowI, ImagePtr IFFTout, ImagePtr resultPtr);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap phase of I1 with I2.
// (I1_mag, I2_phase) -> II1
// (I1_phase, I2_mag) -> II2
//
void
HW_swapPhase(ImagePtr I1, ImagePtr I2, ImagePtr II1, ImagePtr II2)
{
	// dimensions of I1 for the first image width, height, and area.
	int w1 = I1->width();
	int h1 = I1->height();
	int total1 = w1 * h1;

	// dimensions of I2 for the second image width, height, and area.
	int w2 = I2->width();
	int h2 = I2->height();
	int total2 = w2 * h2;

	// checking the sizes of the two images to make sure they are the same.
	if (w1 != w2 || h1 != h2) {
		printf("Dimensions of I1: %d x %d \nDimensions of I2: %d x %d\nAre not equal\n", w1, h1, w2, h2);
		return;
	}

	// copy image header for width and height of the input image I1 to output image II1 & II2
	IP_copyImageHeader(I1, II1);
	IP_copyImageHeader(I1, II2);

	// compute FFT of I1 and I2:
	// final FFT for Image I1 and I2
	ImagePtr I1FFT;
	ImagePtr I1FFTTemp;
	ImagePtr I2FFT; 
	ImagePtr I2FFTTemp;

	// memory allocation for FFT of Image I1 and I2
	I1FFT->allocImage(w1, h1, FFT_TYPE);
	I1FFTTemp->allocImage(w1, h1, FFT_TYPE);
	I2FFT->allocImage(w2, h2, FFT_TYPE);
	I2FFTTemp->allocImage(w2, h2, FFT_TYPE);

	// FFT of I1 and I2
	fft1DRow(I1, I1FFTTemp);
	fft1DColumn(I1, I1FFTTemp, I1FFT);
	fft1DRow(I2, I2FFTTemp);
	fft1DColumn(I2, I2FFTTemp, I2FFT);

	// computing magnitude and phase from real and imaginary FFT channels
	// declaration for the magnitude and phase for Image 1 and Image 2
	float *magnitude1 = new float[total1];             
	float *phase1 = new float[total1];         
	float *magnitude2 = new float[total2];
	float *phase2 = new float[total2];
	fft1DMagPhase(I1, I1FFT, magnitude1, phase1);
	fft1DMagPhase(I2, I2FFT, magnitude2, phase2);

	// image pointers to hold IFFT for FFT fo I1
	// taking the phase of I2 and placing it to I1
	ImagePtr I1IFFTTemp;
	ImagePtr I1Ifft;
	swapPhase(I1FFT, magnitude1, phase2, total1);

	// computing the IFFT for FFT of I1 to restore the image
	Ifft1DRow(I1, I1FFT, I1IFFTTemp);
	Ifft1DColumn(I1, I1IFFTTemp, I1Ifft, II1);

	// image pointers to hold IFFT for FFT fo I2
	// taking the phase of I2 and placing it to I2
	ImagePtr I2IFFTTemp;
	ImagePtr I2Ifft;
	swapPhase(I2FFT, magnitude2, phase1, total2);

	// computing the IFFT for FFT of I2 to restore the image
	Ifft1DRow(I2, I2FFT, I2IFFTTemp);
	Ifft1DColumn(I2, I2IFFTTemp, I2Ifft, II2);
}

// swapPhase function where it takes image I and places the newPhase array as its phase
void swapPhase(ImagePtr I, float * magnitude, float * newPhase, int total) {
	ChannelPtr<float> real, img;
	//instantiating a real and imaginary pointer
	real = I[0]; 
	img = I[1]; 
	
	// updating the real and imaginary parts with the new phase
	for (int i = 0; i < total; ++i) {
		*real++ = magnitude[i] * cos(newPhase[i]);
		*img++ = magnitude[i] * sin(newPhase[i]);
	}
}

void Ifft1DRow(ImagePtr I, ImagePtr IFFT, ImagePtr IFFTout) {
	int w = I->width();
	int h = I->height();
	ChannelPtr<float> real, img, real2, img2;

	// memory allocation for output of FFT1D
	IFFTout->allocImage(w, h, FFT_TYPE);
	real = IFFT[0]; 
	img = IFFT[1]; 
	real2 = IFFTout[0];
	img2 = IFFTout[1];

	// for q1, instantiating the length of the complex number list, setting the pointer to the real number list, setting the pointer to point to the imaginary number list
 	// for q2, instantiating the length of the complex number list, setting the pointer to the real number list, setting the pointer to point to the imaginary number list
	complexP c1, c2, *q1, *q2;
	q1 = &c1;
	q2 = &c2;
	q1->len = w;                           
	q1->real = new float[w];               
	q1->imag = new float[w];             
	q2->len = w;                  
	q2->real = new float[w];
	q2->imag = new float[w];             

	for (int row = 0; row < h; ++row) {
		for (int column = 0; column < w; ++column)
		{
			q1->real[column] = *real++;
			q1->imag[column] = *img++;
		}
		// calling fft1D with dir paramenter as 1
		fft1D(q1, 1, q2); 

		// setting the real output to Image and setting the imaginary output to Image1
		for (int i = 0; i < q2->len; ++i)
		{
			*real2++ = q2->real[i];              
			*img2++ = q2->imag[i];          
		}
	}
}

void Ifft1DColumn(ImagePtr I, ImagePtr IFFT, ImagePtr IFFTout, ImagePtr II1) {
	int w = I->width();
	int h = I->height();
	ChannelPtr<float> real, img, real2, img2;
	ChannelPtr<uchar> pII1;
	int type;

	// memory allocation for the output of FFT1D row
	// instantiating a real and imaginary pointer
	IFFTout->allocImage(w, h, FFT_TYPE);
	real2 = IFFTout[0]; 
	img2 = IFFTout[1]; 

	for (int ch = 0; IP_getChannel(II1, ch, pII1, type); ++ch)
	{
		real = IFFT[0];
		img = IFFT[1];

		// for q1, instantiating the length of the complex number list, setting the pointer to the real number list, setting the pointer to point to the imaginary number list
		// for q2, instantiating the length of the complex number list, setting the pointer to the real number list, setting the pointer to point to the imaginary number list
		complexP c1, c2, *q1, *q2;
		q1 = &c1;
		q2 = &c2;
		q1->len = w;                           
		q1->real = new float[w];                
		q1->imag = new float[w];              
		q2->len = w;                       
		q2->real = new float[w];  
		q2->imag = new float[w];  

		for (int column = 0; column < w; ++column)
		{
			// setting pointers to real and img respectively
			ChannelPtr<float> temp_real = real; 
			ChannelPtr<float> temp_img = img;  
			for (int row = 0; row < h; ++row)
			{
				q1->real[row] = *temp_real;
				q1->imag[row] = *temp_img;

				if (row < h - 1)
				{
					temp_real += w;
					temp_img += w;
				}
			}
			// calling fft1D with dir paramenter as 1
			fft1D(q1, 1, q2);

			// writing the final output image
			for (int i = 0; i < q2->len; ++i)
			{
				//Writing to final output image
				*pII1++ = CLIP(q2->real[i], 0, MaxGray);
			}

			// incrementing th real and img values to the next column
			real++;                  
			img++;     
		}
	}

}