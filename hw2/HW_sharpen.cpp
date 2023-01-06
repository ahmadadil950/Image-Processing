#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//

//Invokes HW_blur with filter dimension size
void HW_blur(ImagePtr I1, int filterWidth, int filterHeight, ImagePtr I2);

void HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
    // PUT YOUR CODE HERE

    //Channel Pointers for keeping track of various image location
    ChannelPtr<uchar> input, output, blur;
    ImagePtr I3; //Pointer used to store blur 
    IP_copyImageHeader(I1, I2); //copy image header to I2 which is the output pointer
    int width = I1->width();
    int height = I1->height();
    int total = width * height; //Holds total input dimenion value
    int type; //Holds datatype
    
    HW_blur(I1, size, size, I3); //Blurs image and stores it in I3
    for (int i = 0; IP_getChannel(I1, i, input, type); i++){ //Iterates through input pointer
        IP_getChannel(I2, i, output, type);// gets current pointer for output channel
		IP_getChannel(I3, i, blur, type); // gets current pointer for blur channel
		for (int j = 0; j < total; j++) {
			*input++, *blur++;
            //Equation for sharpen.Also ensures pixel value is between 0 and 255
            *output = CLIP(*input + (*input - *blur) * factor, 0, MaxGray);
            *output++;
        }   
    }
}
