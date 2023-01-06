// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2021 by George Wolberg
//
// Trapezoid.cpp - Trapezoid transformation widget.
//
// Written by: George Wolberg, 2021
// ======================================================================

#include "MainWindow.h"
#include "Trapezoid.h"

extern MainWindow *g_mainWindowP;
extern void HW_trapezoid(ImagePtr I1, ImagePtr Icorr, ImagePtr I2);
enum { WSIZE, HSIZE, STEPX, STEPY, KERNEL, SAMPLER };

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Trapezoid::Trapezoid:
//
// Constructor.
//
Trapezoid::Trapezoid(QWidget *parent) : ImageFilter(parent)
{
	m_corr   = NULL;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Trapezoid::controlPanel:
//
// Create group box for control panel.
//
QGroupBox*
Trapezoid::controlPanel()
{
	// init group box
	m_ctrlGrp = new QGroupBox("Trapezoid");

	// layout for assembling filter widget
	QVBoxLayout *vbox = new QVBoxLayout;

	// create file pushbutton
	m_button = new QPushButton("File");

	// create text edit widget
	m_values = new QTextEdit();
	m_values->setReadOnly(1);

	// assemble dialog
	vbox->addWidget(m_button);
	vbox->addWidget(m_values);
	m_ctrlGrp->setLayout(vbox);

	// init signal/slot connections
	connect(m_button, SIGNAL(clicked()), this, SLOT(load()));

	return(m_ctrlGrp);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Trapezoid::applyFilter:
//
// Run filter on the image, transforming I1 to I2.
// Overrides ImageFilter::applyFilter().
// Return 1 for success, 0 for failure.
//
bool
Trapezoid::applyFilter(ImagePtr I1, bool gpuFlag, ImagePtr I2)
{
	// error checking
	if(I1.isNull())		return 0;
	if(m_corr.isNull())	return 0;

	// get input image dimensions
	m_width  = I1->width();
	m_height = I1->height();

	// transform image
	if(!(gpuFlag && m_shaderFlag))
		trapezoid(I1, m_corr, I2);
	else    g_mainWindowP->glw()->applyFilterGPU(m_nPasses, I1->width(), I1->height());

	return 1;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Trapezoid::trapezoid:
//
// Apply a trapezoid transformation to I1.
// The four-corner correspondences of the input image is given in Icorr.
// Output is in I2.
//
void
Trapezoid::trapezoid(ImagePtr I1, ImagePtr Icorr, ImagePtr I2)
{
	HW_trapezoid(I1, Icorr, I2);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Trapezoid::load:
//
// Slot to load filter kernel from file.
//
int
Trapezoid::load()
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
		s.append(QString("Error: Trapezoid::load() expected 4 lines of (x,y) numbers\n"));
		m_values->append(s);
		return 1;
	}

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

	// apply filter to source image and display result
	g_mainWindowP->preview();

	return 1;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Trapezoid::initShader:
//
// init shader program and parameters.
//
void
Trapezoid::initShader() 
{
	// number of passes in filter cascade
	m_nPasses = 1;

	// initialize GL function resolution for current context
	initializeGLFunctions();

	// init uniform hash table based on uniform variable names and location IDs
	UniformMap uniforms;
	uniforms["u_Wsize"  ] = WSIZE;
	uniforms["u_Hsize"  ] = HSIZE;
	uniforms["u_StepX"  ] = STEPX;
	uniforms["u_StepY"  ] = STEPY;
	uniforms["u_Kernel" ] = KERNEL;
	uniforms["u_Sampler"] = SAMPLER;

        QString v_name = ":/vshader_passthrough.glsl";
        QString f_name = ":/fshader_convolve.glsl";
 
	// compile shader, bind attribute vars, link shader, and initialize uniform var table
	g_mainWindowP->glw()->initShader(m_program[PASS1], 
	                                 v_name, 
	                                 f_name,
					 uniforms,
					 m_uniform[PASS1]);

	// flag to indicate shader availability
	m_shaderFlag = true;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Trapezoid::gpuProgram:
//
// Invoke GPU program
//
void
Trapezoid::gpuProgram(int pass) 
{
	// collect parameters
	int w_size   = m_corr->width();
	int h_size   = m_corr->height();
	ChannelPtr<float> p;
	int type;
	IP_getChannel(m_corr, 0, p, type);

	// pass parameters to shader
	glUseProgram(m_program[pass].programId());
	glUniform1i (m_uniform[pass][WSIZE], w_size);
	glUniform1i (m_uniform[pass][HSIZE], h_size);
	glUniform1fv(m_uniform[pass][KERNEL], 128, (GLfloat *)&p[0]);
	glUniform1f (m_uniform[pass][STEPX], (GLfloat) 1.0f / m_width);
	glUniform1f (m_uniform[pass][STEPY], (GLfloat) 1.0f / m_height);
	glUniform1i (m_uniform[pass][SAMPLER], 0);
}
