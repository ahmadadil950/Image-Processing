// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2021 by George Wolberg
//
// Perspective.cpp - Perspective transformation widget.
//
// Written by: George Wolberg, 2021
// ======================================================================

#include "MainWindow.h"
#include "Perspective.h"

extern MainWindow *g_mainWindowP;
extern void HW_perspective	(ImagePtr I1, ImagePtr Imatrix, ImagePtr I2);
extern void HW_perspectiveFant	(ImagePtr I1, ImagePtr Imatrix, ImagePtr I2);
enum { PERS, STEPX, STEPY, SCALEX, SCALEY, XMIN, YMIN, SAMPLER };

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::Perspective:
//
// Constructor.
//
Perspective::Perspective(QWidget *parent) : ImageFilter(parent)
{
	m_corr   = NULL;
	m_matrix = NULL;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::controlPanel:
//
// Create group box for control panel.
//
QGroupBox*
Perspective::controlPanel()
{
	// init group box
	m_ctrlGrp = new QGroupBox("Perspective");

	// layout for assembling filter widget
	QVBoxLayout *vbox = new QVBoxLayout;

	// create file pushbutton
	m_button = new QPushButton("File");

	// create text edit widget
	m_values = new QTextEdit();
	m_values->setReadOnly(1);

	// create antialiasing checkbox
	m_antialias = new QCheckBox("Antialiasing");
	m_antialias -> setCheckState(Qt::Unchecked);
	m_antialiasFlag =  m_antialias->isChecked();

	// assemble dialog
	vbox->addWidget(m_button);
	vbox->addWidget(m_values);
	vbox->addWidget(m_antialias);
	m_ctrlGrp->setLayout(vbox);

	// init signal/slot connections
	connect(m_button,    SIGNAL(clicked()),		this, SLOT(load()));
	connect(m_antialias, SIGNAL(stateChanged(int)), this, SLOT(setAntialiasFlag(int)));

	return(m_ctrlGrp);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::applyFilter:
//
// Run filter on the image, transforming I1 to I2.
// Overrides ImageFilter::applyFilter().
// Return 1 for success, 0 for failure.
//
bool
Perspective::applyFilter(ImagePtr I1, bool gpuFlag, ImagePtr I2)
{
	// error checking
	if(I1.isNull())		return 0;
	if(m_matrix.isNull())	return 0;

	// get input image dimensions
	m_width  = I1->width();
	m_height = I1->height();

	// transform image
	if(!(gpuFlag && m_shaderFlag))
		perspective(I1, m_matrix, I2);
	else {
		// collect parameters
		int w = m_width;
		int h = m_height;

		// get pointer to 3x3 matrix elements
		ChannelPtr<float> a;
		int type;
		IP_getChannel(m_matrix, 0, a, type);
		float x, xmax;

		m_xmin = xmax = X(a, 0, 0);
		x = X(a, w, 0);	m_xmin = MIN(m_xmin, x); xmax = MAX(xmax, x);
		x = X(a, w, h);	m_xmin = MIN(m_xmin, x); xmax = MAX(xmax, x);
		x = X(a, 0, h);	m_xmin = MIN(m_xmin, x); xmax = MAX(xmax, x);

		float y, ymax;
		m_ymin = ymax = Y(a, 0, 0);
		y = Y(a, w, 0);	m_ymin = MIN(m_ymin, y); ymax = MAX(ymax, y);
		y = Y(a, w, h);	m_ymin = MIN(m_ymin, y); ymax = MAX(ymax, y);
		y = Y(a, 0, h);	m_ymin = MIN(m_ymin, y); ymax = MAX(ymax, y);

		// bounding box dimensions for output image
		m_outWidth  = CEILING(xmax) - FLOOR(m_xmin);
		m_outHeight = CEILING(ymax) - FLOOR(m_ymin);
		
		// delete buffers that allocated based on input image size
		g_mainWindowP->glw()->context();
		g_mainWindowP->glw()->deleteBuffers();

		// allocate new buffers for output perspective size
		g_mainWindowP->glw()->allocateTextureFBO(m_outWidth, m_outHeight);

		int wFrame = g_mainWindowP->glFrameW();
		int hFrame = g_mainWindowP->glFrameH();
		g_mainWindowP->glw()->setViewport(m_outWidth, m_outHeight, wFrame, hFrame);
		
		inverse3x3(&a[0], m_invP);

		g_mainWindowP->glw()->applyFilterGPU(m_nPasses, m_outWidth, m_outHeight);
	}

	return 1;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::gpuProgram:
//
// Invoke GPU program
//
void
Perspective::gpuProgram(int pass)
{

	// pass parameters to shader
	glUseProgram(m_program[pass].programId());

	glUniform1fv(m_uniform[pass][PERS], 9, (GLfloat *)&m_invP[0]);
	glUniform1f (m_uniform[pass][STEPX], (GLfloat) 1.0f / m_width);
	glUniform1f (m_uniform[pass][STEPY], (GLfloat) 1.0f / m_height);
	glUniform1f (m_uniform[pass][SCALEX], (GLfloat)m_width/m_outWidth);
	glUniform1f (m_uniform[pass][SCALEY], (GLfloat)m_height/m_outHeight);
	glUniform1f (m_uniform[pass][XMIN], (GLfloat)m_xmin);
	glUniform1f (m_uniform[pass][YMIN], (GLfloat)m_ymin);
	glUniform1i (m_uniform[pass][SAMPLER], 0);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::perspective:
//
// Apply a perspective transformation to I1.
// The four-corner mapping of the input image is given in Icorners.
// The 3x3 perspective matrix is given in Imatrix.
// Output is in I2.
//
void
Perspective::perspective(ImagePtr I1, ImagePtr Imatrix, ImagePtr I2)
{
	if(!m_antialiasFlag)
		HW_perspective    (I1, Imatrix, I2);
	else	HW_perspectiveFant(I1, Imatrix, I2);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::load:
//
// Slot to load filter kernel from file.
//
int
Perspective::load()
{
	QFileDialog dialog(this);

	// open the last known working directory
	if(!m_currentDir.isEmpty())
		dialog.setDirectory(m_currentDir);

	// display existing files and directories
	dialog.setFileMode(QFileDialog::ExistingFile);

	// invoke native file browser to select file
	m_file =  dialog.getOpenFileName(this,
		"Open File", m_currentDir,
		"Images (*.AF);;All files (*)");

	// verify that file selection was made
	if(m_file.isNull()) return 0;

	// save current directory
	QFileInfo f(m_file);
	m_currentDir = f.absolutePath();

	// the transformation is specified using four-point correspondences between input corners to output;
	// read the four (x,y) output coordinates for each input corner in clockwise order from top left
	m_corr = IP_readImage(qPrintable(m_file));

	// error checking
	QString s;
	if(m_corr->width() != 2 || m_corr->height() != 4) {
		m_values->clear();
		s.append(QString("Error: Perspective::load() expected 4 lines of (x,y) numbers\n"));
		m_values->append(s);
		return 1;
	}

	// initialize 3x3 perspective transformation matrix
	m_matrix = IP_allocImage(3, 3, FLOATCH_TYPE);
	initMatrix(g_mainWindowP->imageIn(), m_corr, m_matrix);

	// update button with filename (without path)
	m_button->setText(f.fileName());
	m_button->update();

	// get dimensions of input image
	int w = g_mainWindowP->imageSrc()->width();
	int h = g_mainWindowP->imageSrc()->height();

	// get pointer to 4-point correspondences
	int type;
	ChannelPtr<float> p;
	IP_getChannel(m_corr, 0, p, type);

	// display 4-point correspondences
	m_values->clear();			// clear text edit field (kernel values)
	s.append(QString("Input to Output Mapping:\n"));
	s.append(QString("(%1,%2) --> (%3,%4)\n").arg(0).arg(0).arg(*p++).arg(*p++));
	s.append(QString("(%1,%2) --> (%3,%4)\n").arg(w).arg(0).arg(*p++).arg(*p++));
	s.append(QString("(%1,%2) --> (%3,%4)\n").arg(w).arg(h).arg(*p++).arg(*p++));
	s.append(QString("(%1,%2) --> (%3,%4)\n").arg(0).arg(h).arg(*p++).arg(*p++));
	m_values->append(s);		// append string to text edit widget

	// get 3x3 perspective transformation matrix to map input to output
	IP_getChannel(m_matrix, 0, p, type);

	// display 3x3 perspective transformation matrix
	m_values->clear();			// clear text edit field (kernel values)
	s.append(QString("\nMatrix M in [x y 1][M]:\n"));
	s.append(QString("%1\t%2\t%3\n").arg(*p++).arg(*p++).arg(*p++));
	s.append(QString("%1\t%2\t%3\n").arg(*p++).arg(*p++).arg(*p++));
	s.append(QString("%1\t%2\t%3\n").arg(*p++).arg(*p++).arg(*p++));
	m_values->append(s);		// append string to text edit widget

	// apply filter to source image and display result
	g_mainWindowP->preview();

	return 1;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::setAntialiasFlag:
//
// Slot to process state change of Perspective antialias checkbox.
//
void
Perspective::setAntialiasFlag(int state)
{
	m_antialiasFlag = (state == Qt::Checked) ? 1 : 0;

	// apply filter to source image and display result
	g_mainWindowP->preview();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::initShader:
//
// Init shader program and parameters.
//
void
Perspective::initShader() 
{
	// number of passes in filter cascade
	m_nPasses = 1;

	// initialize GL function resolution for current context
	initializeGLFunctions();

	// init uniform hash table based on uniform variable names and location IDs
	UniformMap uniforms;

	uniforms["u_Perspective"] = PERS;
	uniforms["u_StepX"]       = STEPX;
	uniforms["u_StepY"]       = STEPY;
	uniforms["u_Xscale"]	  = SCALEX;
	uniforms["u_Yscale"]	  = SCALEY;
	uniforms["u_Xmin"]        = XMIN;
	uniforms["u_Ymin"]        = YMIN;
	uniforms["u_Sampler"]     = SAMPLER;

        QString v_name = ":/vshader_passthrough.glsl";
        QString f_name = ":/fshader_perspective.glsl";

	// compile shader, bind attribute vars, link shader, and initialize uniform var table
	g_mainWindowP->glw()->initShader(m_program[PASS1], 
	                                 v_name, 
	                                 f_name,
					 uniforms,
					 m_uniform[PASS1]);

	// flag to indicate shader availability
	m_shaderFlag = true;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Perspective::initMatrix:
//
// Given Icorr, a list of the 4 correspondence points for the corners
// of image I, compute the 3x3 perspective matrix in Imatrix.
//
void
Perspective::initMatrix(ImagePtr I, ImagePtr Icorr, ImagePtr Imatrix)
{
	// init pointers
	int type;
	ChannelPtr<float> a, p;
	IP_getChannel(Icorr,   0, p, type);
	IP_getChannel(Imatrix, 0, a, type);

	// init u,v,x,y vars
	float	x0, x1, x2, x3;
	float	y0, y1, y2, y3;
	x0 = *p++;	y0 = *p++;
	x1 = *p++;	y1 = *p++;
	x2 = *p++;	y2 = *p++;
	x3 = *p++;	y3 = *p++;

	int w = I->width ();
	int h = I->height();

	// compute auxiliary vars
	float	dx1, dx2, dx3, dy1, dy2, dy3;
	dx1 = x1 - x2;
	dx2 = x3 - x2;
	dx3 = x0 - x1 + x2 - x3;
	dy1 = y1 - y2;
	dy2 = y3 - y2;
	dy3 = y0 - y1 + y2 - y3;

	// compute 3x3 transformation matrix:
	// a0 a1 a2
	// a3 a4 a5
	// a6 a7 a8
	//
	float	a13, a23;
	a13  = (dx3*dy2 - dx2*dy3) / (dx1*dy2 - dx2*dy1);
	a23  = (dx1*dy3 - dx3*dy1) / (dx1*dy2 - dx2*dy1);
	a[0] = (x1-x0+a13*x1) / w;
	a[1] = (y1-y0+a13*y1) / w;
	a[2] = a13 / w;
	a[3] = (x3-x0+a23*x3) / h;
	a[4] = (y3-y0+a23*y3) / h;
	a[5] = a23 / h;
	a[6] = x0;
	a[7] = y0;
	a[8] = 1;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// inverse3x3:
//
// Find the inverse p2 of a 3x3 perspecive matrix p1.
// Return 1 if there is an inverse; else return 0.
//
int
inverse3x3(float *p1, float *p2)
{
	// matrix determinant
	double det = p1[0] * (p1[4] * p1[8] - p1[7] * p1[5])
		   - p1[1] * (p1[3] * p1[8] - p1[5] * p1[6])
		   + p1[2] * (p1[3] * p1[7] - p1[4] * p1[6]);

	// if determinant is 0, there is no inverse
	if(det == 0) {
		// set p2 to identity matrix to avoid errors 
		for(int i=0; i<9; i++) p2[i] = 0;	// clear elements
		p2[0] = p2[4] = p2[8] = 1;		// set diagonal to 1
		return 0;
	}

	// evaluate matrix elements
	p2[0] = ( p1[4] * p1[8] - p1[7] * p1[5]) / det;
	p2[1] = (-p1[1] * p1[8] + p1[2] * p1[7]) / det;
	p2[2] = ( p1[1] * p1[5] - p1[4] * p1[2]) / det;
	p2[3] = (-p1[3] * p1[8] + p1[5] * p1[6]) / det;
	p2[4] = ( p1[0] * p1[8] - p1[6] * p1[2]) / det;
	p2[5] = (-p1[5] * p1[0] + p1[2] * p1[3]) / det;
	p2[6] = ( p1[3] * p1[7] - p1[4] * p1[6]) / det;
	p2[7] = (-p1[7] * p1[0] + p1[1] * p1[6]) / det;
	p2[8] = ( p1[0] * p1[4] - p1[1] * p1[3]) / det;

	// normalize so that p2[8] = 1
	for(int i=0; i<9; i++) p2[i] /= p2[8];

	return 1;
}
