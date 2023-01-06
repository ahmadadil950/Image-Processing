#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//
void blur_1D(ChannelPtr<uchar>, int, int, int, ChannelPtr<uchar>);
void HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{

	// PUT YOUR CODE HERE
	/**
     * Initialize function copy image header
     * for width and height of input image(I1) to output image(I2)
     */
    IP_copyImageHeader(I1, I2);

	ImagePtr I3;

	IP_copyImageHeader(I1,I3);

    /**
     * building our width and height for the image 
     * along with the total number of pixles within the image
     */
    int width = I1 -> width();
    int height = I1 -> height();
    int total = width * height;

	int type;
	ChannelPtr<uchar> p1,p2,p3,end;

	for(int ch = 0; IP_getChannel(I1,ch,p1,type); ch++){
		IP_getChannel(I2,ch,p2,type);
		IP_getChannel(I3,ch,p3,type);

		if(filterW == 1){
			for(end = p1 + total; p1< end;)
				*p3++ = *p1++;
		}
		
		if (filterW > 1) {
            // blur rows one by one
				for (int y = 0; y < height; y++) {
					blur_1D(p1, width, 1, filterW, p3);
					p1 += width;
					p3 += width;
				}
        	}

			p3 = p3 - total;

			if(filterH == 1){
				for(end = p3 + total; p3< end;)
						*p2++ = *p3++;
			}

			if(filterH > 1){
				for(int x = 0; x < width; x++)
				{
					blur_1D(p3,height,width,filterH,p2);
					p3 += 1;
					p2 += 1;
				}
			}

		}
}

void blur_1D(ChannelPtr<uchar> source, int len, int stride, int w, ChannelPtr<uchar> DST)
{
	/**
	 * set w to always be an odd number
	 * sets how many pixels on the left and right
	 * len for padded buffer
	 * buffer to store a padded rows or columns
	 * sets an index for source
	 * sets sum for all pixels
	 */
	if(w % 2 == 0) w++;
	int neighbor_SZ = w/2;
	int pad_buff = len + w -1;
	short* buffer = new short[pad_buff];
	int index = 0;
	unsigned short sum = 0;

	// copy to buffer

	//copy first pixel to left padded area
	for(int i = 0; i<neighbor_SZ; i++){
		buffer[i] = *source;
	}
	// continue with pixel replication
	for(int i = neighbor_SZ; i < len + neighbor_SZ; i++){
		buffer[i] = source[index];
		// next index
		index += stride;
	}
	// copy last pixel to right padded area
	for(int i = len + neighbor_SZ; i < pad_buff; i++){
		buffer[i] = source[index - stride];
	}


	for(int i = 0; i < w; i++){
		sum += buffer[i];
	}
	for(int i = 0; i<len; i++){
		DST[i * stride] = sum / w;
		sum += (buffer[i + w] - buffer[i]);
	}

	delete[] buffer;

}
