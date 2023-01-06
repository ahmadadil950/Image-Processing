// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2021 by George Wolberg
//
// Perspective.h - Perspective transformation widget
//
// Written by: George Wolberg, 2021
// ======================================================================

#ifndef PERSPECTIVE_H
#define PERSPECTIVE_H

#include "ImageFilter.h"

#define X(A, U, V)	((A[0]*U + A[3]*V + A[6]) / (A[2]*U + A[5]*V + A[8]))
#define Y(A, U, V)	((A[1]*U + A[4]*V + A[7]) / (A[2]*U + A[5]*V + A[8]))
#define H(A, X, V)	((-(A[5]*V+A[8])*X+ A[3]*V + A[6]) / (A[2]*X - A[0]))
#define TRIFILTER(A,B,X) (A*(1-X) + B*X)
int	inverse3x3	(float*, float*);

class Perspective : public ImageFilter {
	Q_OBJECT

public:
	Perspective			(QWidget *parent = 0);	// constructor
	QGroupBox*	controlPanel	();			// create control panel
	bool		applyFilter	(ImagePtr, bool, ImagePtr);	// apply filter to input
	void		perspective	(ImagePtr, ImagePtr, ImagePtr);
	void		initMatrix	(ImagePtr, ImagePtr, ImagePtr);
	void		initShader	();
	void		gpuProgram(int pass);	// use GPU program to apply filter

protected slots:
	int		load		();
	void		setAntialiasFlag(int);

private:
	// widgets
	QPushButton*	m_button;	// Perspective pushbutton
	QTextEdit*	m_values;	// text field for mapping and matrix values
	QGroupBox*	m_ctrlGrp;	// groupbox for panel
	QCheckBox*	m_antialias;	// antialiasing checkbox
	int		m_antialiasFlag;// antialiasing checkbox flag

	// variables
	QString		m_file;
	QString		m_currentDir;
	ImagePtr	m_corr;		// 2x4 array of output coordinates for input image corners
	ImagePtr	m_matrix;	// 3x3 perspective transformation matrix
	int		m_width;	// input image width
	int		m_height;	// input image height
	int		m_outWidth;	// output width
	int		m_outHeight;	// output height
	float		m_invP[9];	// inverse perspective transform
	float		m_xmin;
	float		m_ymin;
};

#endif	// PERSPECTIVE_H
