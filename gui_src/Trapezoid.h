// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2021 by George Wolberg
//
// Trapezoid.h - Trapezoid transformation widget
//
// Written by: George Wolberg, 2021
// ======================================================================

#ifndef TRAPEZOID_H
#define TRAPEZOID_H

#include "ImageFilter.h"

class Trapezoid : public ImageFilter {
	Q_OBJECT

public:
	Trapezoid			(QWidget *parent = 0);	// constructor
	QGroupBox*	controlPanel	();			// create control panel
	bool		applyFilter	(ImagePtr, bool, ImagePtr);	// apply filter to input
	void		trapezoid	(ImagePtr, ImagePtr, ImagePtr);
	void		initShader	();
	void		gpuProgram(int pass);	// use GPU program to apply filter

protected slots:
	int		load		();

private:
	// widgets
	QPushButton*	m_button;	// Trapezoid pushbutton
	QTextEdit*	m_values;	// text field for mapping and matrix values
	QGroupBox*	m_ctrlGrp;	// groupbox for panel

	// variables
	QString		m_file;
	QString		m_currentDir;
	ImagePtr	m_corr;		// 2x4 array of output coordinates for input image corners
	int		m_width;	// input image width
	int		m_height;	// input image height
};

#endif	// TRAPEZOID_H
