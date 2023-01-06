#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_gammaCorrect:
//
// Gamma correct image I1. Output is in I2.
//
void
HW_gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
{

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


    int i, lut[MXGRAY];

     /**
      * Start running loop from 0 to 255
      * fix Grayscale to a value Between 0 and 1 
      * Then Apply Gamma Correction 
      * Then Multiply Back to 255 For Grayscale
      */
    for(i =0; i<MXGRAY; i++)
    {
        lut[i] = pow(i / 255.0, 1/gamma) * 255;
    }


    // Declarations used for image channel pointers
    ChannelPtr<uchar> p1, p2;
    int type;

    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	 // get input  pointer for channel ch
          IP_getChannel(I2, ch, p2, type);		                // get output pointer for channel ch
          for (i = 0; i < total; i++) *p2++ = lut[*p1++];	      // use lut[] to eval output
     }

}
