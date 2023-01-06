#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_clip:
//
// Clip intensities of image I1 to [t1,t2] range. Output is in I2.
// If    input<t1: output = t1;
// If t1<input<t2: output = input;
// If      val>t2: output = t2;
//
void
HW_clip(ImagePtr I1, int t1, int t2, ImagePtr I2)
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


    /**
     * Initalizing the lookup table values
     * for a better approach for clipping
     * 
     * first run loop From 0 to 255 to Set Up LUT
     * 
     * set LUT[i] = i as the default 
     * t1< input < t2: output = input
     * 
     * first condition:
     * if lut[i] < t1, Clip at t1. For input < t1: output = t1;
     * 
     * second condition:
     * if lut[i] > t2, Clip at t2. For val > t2: output = t2;
     * 
     * else
     * set LUT[i] = i
     */
    int i, lut[MXGRAY];

    for(i = 0; i < MXGRAY; i++)
    {
        lut[i] = i;
        if(lut[i] < t1)
        {
            lut[i] = t1;
        }
        else if (lut[i] > t2)
        {
            lut[i] = t2;
        }
        else
        {
            lut[i] = i;
        }
    }
    
    // Declarations used for image channel pointers
    ChannelPtr<uchar> p1, p2;
    int type;

    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	 // get input  pointer for channel ch
          IP_getChannel(I2, ch, p2, type);		                // get output pointer for channel ch
          for (i = 0; i < total; i++) *p2++ = lut[*p1++];	      // use lut[] to eval output
     }

}
