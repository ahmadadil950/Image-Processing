#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
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
    
    // Initalizations for channel pointers 
    int type;
    ChannelPtr<uchar> p1, p2;
    

    /**
     * t1 == t2 case
     * making the output the same as the input
     * 
     */
    if(t1 == t2)
    {
        for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {    // get input  pointer for channel ch
              IP_getChannel(I2, ch, p2, type);                        // get output pointer for channel ch
              for (i = 0; i < total; i++) *p2++ = *p1++;              // use lut[] to eval output
         }
         return;
    }


    /**
     * initalizing the lookup table
     * enable streching from t1 and t2 to fill up from [0 255] values
     */
    for (i = 0; i < MXGRAY; i++)
    {
        lut[i] = (int) CLIP((255.0 * (i-t1)) / (t2-t1), 0, 255);
    }

    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	 // get input  pointer for channel ch
          IP_getChannel(I2, ch, p2, type);		                // get output pointer for channel ch
          for (i = 0; i < total; i++) *p2++ = lut[*p1++];	      // use lut[] to eval output
     }

}
