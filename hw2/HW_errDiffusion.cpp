#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//

void gammaCorrect(ImagePtr, double, ImagePtr);
void copyRow_ToBuffer(ChannelPtr<uchar>, short*, int, int);
void HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{

// PUT YOUR CODE HERE

ImagePtr IG;

IP_copyImageHeader(I1,I2);
IP_copyImageHeader(I1,IG);

gammaCorrect(I1, gamma, IG);

    /**
     * building our width and height for the image 
     * along with the total number of pixles within the image
     */
    int width = I1 -> width();
    int height = I1 -> height();
    int total = width * height;

    int kernalSize = 3;
    int thres = MXGRAY/2;
    int type;
    ChannelPtr<uchar> p1, p2, end;

    if(method == 0){
        short* in1;
        short* in2;
        short e;

        int buffer_size = width + 2;

        short* buffer0 = new short[buffer_size];
        short* buffer1 = new short[buffer_size];

        for(int ch = 0; IP_getChannel(IG, ch, p1, type); ch++)
        {
            IP_getChannel(I2, ch, p2, type);

            copyRow_ToBuffer(p1,buffer0,width,kernalSize);

            p1 = p1 + width;

            for(int i = 1; i < height; i++){
                if(serpentine){
                    if( i % 2==0){
                        copyRow_ToBuffer(p1,buffer0,width,kernalSize);
                        p1 = p1 + width;

                        in1 = buffer1 + width + 1;
                        in2 = buffer0 + width + 1;

                        p2 = p2 + width -1;

                        for(int j = 0; j < width; j++)
                        {
                            *p2 = (*in1 < thres) ? 0 : 255;
                            e = *in1 - *p2;
                            *(in1 - 1) += (e * 7 / 16.0);
                            *(in2 - 1) += (e * 3 / 16.0);
                            *(in2) += (e * 5 / 16.0);
                            *(in2 - 1) += (e * 1 / 16.0);

                            in1--;
                            in2--;
                            p2--;
                        }
                        p2 = p2 + width + 1;
                    }

                    else
                    {
                        copyRow_ToBuffer(p1, buffer1, width, kernalSize);
                        p1 = p1 + width;
                        in1 = buffer0 + 1;
                        in2 = buffer1 + 1;

                        for (int x = 0; x < width; x++) {
                            *p2 = (*in1 < thres) ? 0 : 255;
                            e = *in1 - *p2;
                            *(in1 + 1) += (e * 7 / 16.0);
                            *(in2 - 1) += (e * 3 / 16.0);
                            *(in2) += (e * 5 / 16.0);
                            *(in2 + 1) += (e * 1 / 16.0);

                            in1++;
                            in2++;
                            p2++;
                    }
                    
                }
            }

            else
            {
                if (i % 2 == 0) {
                        copyRow_ToBuffer(p1, buffer0, width, kernalSize);
                        in1 = buffer1 + 1;
                        in2 = buffer0 + 1;
                    }
                    else {
                        copyRow_ToBuffer(p1, buffer1, width, kernalSize);
                        in1 = buffer0 + 1;
                        in2 = buffer1 + 1;
                    }
                    p1 = p1 + width;

                    for (int x = 0; x < width; x++) {
                        *p2 = (*in1 < thres) ? 0 : 255;
                        e = *in1 - *p2;
                        *(in1 + 1) += (e * 7 / 16.0);
                        *(in2 - 1) += (e * 3 / 16.0);
                        *(in2) += (e * 5 / 16.0);
                        *(in2 + 1) += (e * 1 / 16.0);
                        in1++;
                        in2++;
                        p2++;
                    }
            
                }
            }

        }

        delete[] buffer0;
        delete[] buffer1;
    }

    else if (method == 1)
    {
        short** in = new short* [3];
        int e;

        int bufSz = width + 4;
        kernalSize = 5;
        short** buffers = new short* [3];
        for (int i = 0; i < 3; i++) {
            buffers[i] = new short[bufSz];
    }

    for(int ch = 0; IP_getChannel(IG, ch, p1, type); ch++){
        IP_getChannel(I2, ch, p2, type);

            // copy first row to first 2 buffers
            copyRow_ToBuffer(p1, buffers[0], width, kernalSize);
            copyRow_ToBuffer(p1, buffers[1], width, kernalSize);
            p1 = p1 + width;

            for (int y = 2; y < height; y++) {
                if (y % 3 == 0) copyRow_ToBuffer(p1, buffers[0], width, kernalSize);
                else if (y % 3 == 1) copyRow_ToBuffer(p1, buffers[1], width, kernalSize);
                else if (y % 3 == 2) copyRow_ToBuffer(p1, buffers[2], width, kernalSize);
                p1 = p1 + width;

                if (serpentine) { // serpentine scan
                    if (y % 2 == 0) { // on even rows
                        if (y % 3 == 0) {
                            in[0] = buffers[1] + width + 2;
                            in[1] = buffers[2] + width + 2;
                            in[2] = buffers[0] + width + 2;

                        }
                        else if (y % 3 == 1) {
                            in[0] = buffers[2] + width + 2;
                            in[1] = buffers[0] + width + 2;
                            in[2] = buffers[1] + width + 2;

                        }
                        else {
                            in[0] = buffers[0] + width + 2;
                            in[1] = buffers[1] + width + 2;
                            in[2] = buffers[2] + width + 2;
                        }
                         p2 = p2 + width - 1;
                        for (int x = 0; x < width; x++) {
                            *p2 = (*in[0] < thres) ? 0 : 255;
                            e = *in[0] - *p2;
                            *(in[0] - 1) += (e * 7 / 48);
                            *(in[0] - 2) += (e * 5 / 48);
                            *(in[1]) += (e * 7 / 48);
                            *(in[1] + 1) += (e * 5 / 48);
                            *(in[1] + 2) += (e * 3 / 48);
                            *(in[1] - 1) += (e * 5 / 48);
                            *(in[1] - 2) += (e * 3 / 48);
                            *(in[2]) += (e * 5 / 48);
                            *(in[2] + 1) += (e * 3 / 48);
                            *(in[2] + 2) += (e * 1 / 48);
                            *(in[2] - 1) += (e * 3 / 48);
                            *(in[2] - 2) += (e * 1 / 48);
                            in[0]--;
                            in[1]--;
                            in[2]--;
                            p2--;
                        }
                        p2 = p2 + width + 1;
                    }
                    else { // on add rows
                        if (y % 3 == 0) {
                            in[0] = buffers[1] + 2;
                            in[1] = buffers[2] + 2;
                            in[2] = buffers[0] + 2;

                        }
                        else if (y % 3 == 1) {
                            in[0] = buffers[2] + 2;
                            in[1] = buffers[0] + 2;
                            in[2] = buffers[1] + 2;

                        }
                        else {
                            in[0] = buffers[0] + 2;
                            in[1] = buffers[1] + 2;
                            in[2] = buffers[2] + 2;
                        }
                        for (int x = 0; x < width; x++) {
                            *p2 = (*in[0] < thres) ? 0 : 255;
                            e = *in[0] - *p2;
                            *(in[0] + 1) += (e * 7 / 48);
                            *(in[0] + 2) += (e * 5 / 48);
                            *(in[1]) += (e * 7 / 48);
                            *(in[1] + 1) += (e * 5 / 48);
                            *(in[1] + 2) += (e * 3 / 48);
                            *(in[1] - 1) += (e * 5 / 48);
                            *(in[1] - 2) += (e * 3 / 48);
                            *(in[2]) += (e * 5 / 48);
                            *(in[2] + 1) += (e * 3 / 48);
                            *(in[2] + 2) += (e * 1 / 48);
                            *(in[2] - 1) += (e * 3 / 48);
                            *(in[2] - 2) += (e * 1 / 48);
                            in[0]++;
                            in[1]++;
                            in[2]++;
                            p2++;
                        }
            
                    }
    
                }
                else { // raster scan
                    if (y % 3 == 0) {
                        in[0] = buffers[1] + 2;
                        in[1] = buffers[2] + 2;
                        in[2] = buffers[0] + 2;

                    }
                    else if (y % 3 == 1) {
                        in[0] = buffers[2] + 2;
                        in[1] = buffers[0] + 2;
                        in[2] = buffers[1] + 2;

                    }
                    else {
                        in[0] = buffers[0] + 2;
                        in[1] = buffers[1] + 2;
                        in[2] = buffers[2] + 2;

                    }
                    for (int x = 0; x < width; x++) {
                        *p2 = (*in[0] < thres) ? 0 : 255;
                        e = *in[0] - *p2;
                        *(in[0] + 1) += (e * 7 / 48);
                        *(in[0] + 2) += (e * 5 / 48);
                        *(in[1]) += (e * 7 / 48);
                        *(in[1] + 1) += (e * 5 / 48);
                        *(in[1] + 2) += (e * 3 / 48);
                        *(in[1] - 1) += (e * 5 / 48);
                        *(in[1] - 2) += (e * 3 / 48);
                        *(in[2]) += (e * 5 / 48);
                        *(in[2] + 1) += (e * 3 / 48);
                        *(in[2] + 2) += (e * 1 / 48);
                        *(in[2] - 1) += (e * 3 / 48);
                        *(in[2] - 2) += (e * 1 / 48);
                        in[0]++;
                        in[1]++;
                        in[2]++;
                        p2++;
                    }
                }
            }
        }
        delete[] buffers;
    }
    else {
        for (int ch = 0; IP_getChannel(IG, ch, p1, type); ch++) {
            IP_getChannel(I2, ch, p2, type);
            for (end = p1 + width * height; p1 < end;) *p2++ = *p1++;
        }
    }
}

void gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
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

void copyRow_ToBuffer(ChannelPtr<uchar> p1, short* buffer, int width, int size)
{
    int buffer_size = size + width -1;
    for(int i = 0; i < size/2; i++){
        buffer[i] = *p1;
    }
    for(int i = size/2; i<size/2 + width -1; i++){
        buffer[i] = *p1++;
    }
    for(int i = size/2 + width -1; i<buffer_size; i++){
        buffer[i] = *p1;
    }
}
