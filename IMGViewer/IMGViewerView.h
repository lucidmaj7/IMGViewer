
// IMGViewerView.h : interface of the CIMGViewerView class
//

#pragma once
#include <vector>
using namespace std;

class CIMGViewerView : public CView
{
protected: // create from serialization only
	CIMGViewerView();
	DECLARE_DYNCREATE(CIMGViewerView)

// Attributes
public:
	CIMGViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CIMGViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	HBITMAP CIMGViewerView::JpegStreamIn(FILE* infile);
	HBITMAP m_hBitmap;
	void CIMGViewerView::GetFiles(vector<string> &vList, string sPath, bool bAllDirectories);
	vector <string> m_vtfilelist;
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	BOOL CIMGViewerView::NextImgLoad(int _nFileIdx);
	int m_nFileIdx;
	virtual void OnInitialUpdate();
	CStatusBar* m_pStatusBar;
};

#ifndef _DEBUG  // debug version in IMGViewerView.cpp
inline CIMGViewerDoc* CIMGViewerView::GetDocument() const
   { return reinterpret_cast<CIMGViewerDoc*>(m_pDocument); }
#endif

