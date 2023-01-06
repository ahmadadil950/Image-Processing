#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_contrast:
//
// Apply contrast enhancement to I1. Output is in I2.
// Stretch intensity difference from reference value (128) by multiplying
// difference by "contrast" and adding it back to 128. Shift result by
// adding "brightness" value.
//
void
HW_contrast(ImagePtr I1, double brightness, double contrast, ImagePtr I2)
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
     * inialize our lookup table
     * 
     * Utilizing the CLIP function we ensure
     * the streched intensities from the reference value 
     * and brightness as an offset
     * 
     */
    int i, lut[MXGRAY];

    for(i = 0; i<MXGRAY; i++)
    {
        lut[i] = (int) CLIP((contrast * (i - 128))
                 + 128 + brightness,0,MaxGray);
    }

    // Declarations used for image channel pointers
    ChannelPtr<uchar> p1, p2;
    int type;

    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	 // get input  pointer for channel ch
          IP_getChannel(I2, ch, p2, type);		                // get output pointer for channel ch
          for (i = 0; i < total; i++) *p2++ = lut[*p1++];	      // use lut[] to eval output
     }

}
