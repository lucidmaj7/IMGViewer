
// IMGViewer.h : main header file for the IMGViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CIMGViewerApp:
// See IMGViewer.cpp for the implementation of this class
//

class CIMGViewerApp : public CWinApp
{
public:
	CIMGViewerApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CIMGViewerApp theApp;
