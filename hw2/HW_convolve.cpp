#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_convolve:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//
void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2)
{

// PUT YOUR CODE HERE

// PUT YOUR CODE HERE
	/**
     * Initialize function copy image header
     * for width and height of input image(I1) to output image(I2)
     */
    IP_copyImageHeader(I1, I2);

    /**
     * building our width and height for the image 
     * along with the total number of pixles within the image
     */
    int width = I1 -> width();
    int height = I1 -> height();
    int total = width * height;

    ChannelPtr<uchar> in, out, start;
    ChannelPtr<float> kernal, kernalp;
    int type;

    int kernalW = Ikernel->width();
    int kernalH = Ikernel->height();
    int pad_W = kernalW / 2;
    int pad_H = kernalH / 2;

    int maxW = pad_W + width + pad_W;
    int maxH = pad_H + height + pad_H;

    unsigned char **buffer = new unsigned char*[maxH];
    for(int i = 0; i<maxH; i++){
        buffer[i] = new unsigned char[maxW];
    }

    for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {    // get input  pointer for channel ch
          IP_getChannel(I2, ch, out, type);        // get output pointer for channel ch
          start = in; //making start point at the beginning of image (0,0) to use as reference
          for (int i = 0; i < maxH; i++) { // Setting Up Buffer Values, Starting From First Row
               for (int j = 0; j < maxW; j++) { // Setting Up Buffer Values, Starting From First Column
                    if (i < pad_H || i >= pad_H + height || j < pad_W || j >= pad_W + width) { // Buffer Is Outside The Image, The Padding Part
                         buffer[i][j] = 0; // Set Buffer Values To Zero
                    }
                    else { // Inside Image
                         buffer[i][j] = *start++; //  Set Buffer Values To Same As Image Values
                    }
               }
          }

        for (int i = pad_H; i < height + pad_H; i++) { // Work On The Image. Starting From First Row Of The Image
               for (int j = pad_W; j < width + pad_W; j++) { // Work On The Image. Starting From First Column Of The Image
                    double sum = 0; // Reset Sum To Zero Everytime It Loops
                    IP_getChannel(Ikernel, 0, kernalp, type); // Setting Ikernel To kernelP
                    for (int kr = 0; kr < kernalH; kr++) { //All The Rows Of The Kernel
                         for (int kc = 0; kc < kernalW; kc++) { //Each Column of The kernel
                              sum += buffer[i - pad_H + kr][j - pad_W + kc] * (*kernalp++); // Convolve Is The Sum Of The Products. Sum = Buffer[i][j] * KernelP
                         }
                    }
                    *out++ = CLIP(sum, 0, 255); // Output Is The Clip Of The Sum. 
               }
          }
     }

    for(int i = 0; i<maxH; i++){
        delete[] buffer[i];
    }

    delete[] buffer;


}
