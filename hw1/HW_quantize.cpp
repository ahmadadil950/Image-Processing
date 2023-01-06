#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
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
    // Initialize our scale and bias variables along with lookup table
    double scale = (double)MXGRAY/levels;
    double bias = 128/levels;
    for(i = 0; i<MXGRAY; i++)
    {
        lut[i] = (int) CLIP((scale * (int) (i/scale)) + bias,0,255.0);
    }

    // p1 points to I1 channels and p2 to I2 channels
    int type;
    ChannelPtr<uchar> p1,p2;
    
    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
          IP_getChannel(I2, ch, p2, type);		                
          for (i = 0; i < total; i++)
          {
            // Adding noise to the image 
            if(dither)
            {
                /**
                 * 1. normalized random number in range [0...1]
                 * 2. Creates range [-bias ..... bias]
                 * 3. input with noise added
                 * 4. Clipping to correct bounds
                 */
                double r = (double) rand()/RAND_MAX;

                int noise = bias * (1-(2 * r));

                int nValue = (*p1++) + noise;

                if(nValue > 255)
                {
                    nValue = 255;
                }
                if(nValue < 0)
                {
                    nValue = 0;
                }

                *p2++ = lut[nValue];
            }

            else
            {
                *p2++ = lut[(*p1++)];
            }
          }	      
     }


}
