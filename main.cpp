/*********************************************************************
 * Name:      	main.cpp
 * Purpose:   	Implements simple wxWidgets application with GUI.
 * Author:    
 * Created:   
 * Copyright: 
 * License:   	wxWidgets license (www.wxwidgets.org)
 * 
 * Notes:		
 *********************************************************************/
 
#include <wx/wx.h>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

extern "C"
{
#include "jpegdecoder.h"		
}


// application class
class wxMiniApp : public wxApp
{
    public:
		// function called at the application initialization
        virtual bool OnInit();

		// event handler for button click
        //void OnClick(wxCommandEvent& event) { GetTopWindow()->Close(); }
};

class MyCanvas;

class MyFrame : public wxFrame
{
public:	
	MyFrame();

private:
	MyCanvas* m_canvas;
	
};

class MyCanvas : public wxPanel
{
public:
	MyCanvas( MyFrame* parent );
	void OnPaint(wxPaintEvent &event);
	
	virtual ~MyCanvas();
	
#if wxUSE_GRAPHICS_CONTEXT
    void UseGraphicContext(bool use) { m_useContext = use; Refresh(); }
#endif

	DECLARE_EVENT_TABLE()
	
private:

#if wxUSE_GRAPHICS_CONTEXT
    bool         m_useContext ;
#endif
	RawImage* m_image;
};

BEGIN_EVENT_TABLE(MyCanvas, wxPanel)
    EVT_PAINT  (MyCanvas::OnPaint)
END_EVENT_TABLE()


MyFrame::MyFrame()
: wxFrame( NULL, -1, wxT("Test Window"), wxDefaultPosition, wxSize( 1152, 864) )
{
	m_canvas = new MyCanvas( this );
}
	
MyCanvas::MyCanvas( MyFrame* parent )
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
#if wxUSE_GRAPHICS_CONTEXT
    m_useContext = false;
#endif

	
	int ret;
	m_image = NULL;
	ret = read_jpeg_file("../test.jpg", &m_image );
	if( ret == 0 && m_image )
	{
		parent->SetClientSize( m_image->width, m_image->height );
	}
		
}

MyCanvas::~MyCanvas()
{
	if( m_image )
	{
		if( m_image->data )
			delete m_image->data;
		delete m_image;
	}
}

void MyCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	int width,height;
	GetClientSize(&width, &height);
		
	int picDim = width < height?width:height;
	
	picDim /= 8;
	picDim *= 8;
	
	
	printf("%d\n",picDim);
	
	wxPaintDC pdc(this);

#if wxUSE_GRAPHICS_CONTEXT
     wxGCDC gdc( pdc ) ;
     wxDC &dc = m_useContext ? (wxDC&) gdc : (wxDC&) pdc ;
#else
    wxDC &dc = pdc ;
#endif

    PrepareDC(dc);
	
	dc.Clear();
	
	int ii=0;
	int jj= 0;
	int kk = 0;
	int stridewidth = 3 * m_image->width;
	for( ii = 0; ii < m_image->height; ii ++ )
	{
		for( jj = 0; jj < stridewidth; jj+=3 )
		{
			kk = ii*stridewidth;
			dc.SetPen( wxPen(wxColour(m_image->data[kk + jj],m_image->data[kk + jj+1],m_image->data[kk + jj+2])) );
			dc.DrawPoint( jj / 3, ii );
		}
	
	}
		
}

IMPLEMENT_APP(wxMiniApp);

bool wxMiniApp::OnInit()
{
	// create a new frame and set it as the top most application window
    SetTopWindow( new MyFrame() );

	// show main frame
    GetTopWindow()->Show();

	// enter the application's main loop
    return true;
}
