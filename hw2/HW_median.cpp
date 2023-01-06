#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is in I2.
//
void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{

	// PUT YOUR CODE HERE

	// initializing and declaring variables
	IP_copyImageHeader(I1, I2);
	int w = I1->width();
	int h = I1->height();
	int total = w * h;
	uchar *output_img;
	int median;

	// clamping to 9
	if (sz > 9)
		sz = 9;

	// checking if even then increment to the next odd filter
	if (sz % 2 == 0)
		sz++;

	//creating space for the buffer to the max filter size = 9
	uchar *buffer[9];
	for (int i = 0; i < sz; i++) {
		buffer[i] = new uchar[w + (sz - 1)];
	}

	// creating space for row to the max filter size = 9
	uchar *row[9];
	for (int i = 0; i < sz; i++) {
		row[i] = buffer[i] + sz / 2;
	}

	int type, i, j;
	std::vector<uchar> bufferV;
	ChannelPtr<uchar> p1, p2, endd;
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		output_img = (uchar *)&p2[0];


		for (int x = 0; x < w; x++) row[0][x] = *p1++; // copying first row from input to row zero in the buffer
		for (int x = 1; x <= sz / 2; x++) row[0][-x] = row[0][0]; // creating padding to the left side for leftmost pixel
		for (int x = w; x < w + (sz / 2); x++) row[0][x] = row[0][w - 1]; // creating padding to the right side for the rightmost pixel

		// copying row zero to row of sz divided by 2
		for (int i = 0; i <= sz / 2; i++)
			for (j = 0; j < w + ((sz / 2) - 1); j++)
				buffer[i][j] = buffer[0][j];

		// copying the rest of the rows into a buffer
		for (int i = (sz / 2) + 1; i < sz; i++) {
			for (int x = 0; x < w; x++) row[i][x] = *p1++; // copying first row from input to row zero in the buffer
			for (int x = 1; x <= sz / 2; x++) row[i][-x] = row[i][0]; // creating padding to the left side for leftmost pixel
			for (int x = w; x < w + (sz / 2); x++) row[i][x] = row[i][w - 1]; // creating padding to the right side for the rightmost pixel

		}
		// iterating through each pixel of the input image
		int index = 0;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				bufferV.clear();
				for (int i = 0; i < sz; i++) {
					for (int s = -(sz / 2); s <= sz / 2; s++) {
						bufferV.push_back(row[i][x + s]);
					}
				}
				std::sort(bufferV.begin(), bufferV.end()); // sort buffer
				median = bufferV[sz*sz / 2]; // finding the median and storing it
				output_img[index] = median; // copying the median into a corresponding output pixel 
				index++;

			}
			// increases the rows
			for (j = 0; j < sz - 1; j++) row[j] = row[j + 1]; 
			row[j] = buffer[y%sz] + sz / 2;
			if (y < h - sz) {
				for (int x = 0; x < w; x++)
					row[j][x] = *p1++;
			}
		}
	}
}
