
// IMGViewerView.cpp : implementation of the CIMGViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "IMGViewer.h"
#endif
#include <omp.h>      // OpenMP 에 대한 함수가 정의된 헤더파일
#include "IMGViewerDoc.h"
#include "IMGViewerView.h"
#include "jpeglib.h"
#ifndef WIDTHBYTES
#define WIDTHBYTES(bits) (((bits)+31)/32*4)
#endif//WIDTHBYTES

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIMGViewerView

IMPLEMENT_DYNCREATE(CIMGViewerView, CView)

BEGIN_MESSAGE_MAP(CIMGViewerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CIMGViewerView construction/destruction

CIMGViewerView::CIMGViewerView()
{
	// TODO: add construction code here
	m_vtfilelist.clear();
	m_hBitmap = NULL;
	m_nFileIdx = -1;
}

CIMGViewerView::~CIMGViewerView()
{
}
void CIMGViewerView::GetFiles(vector<string> &vList, string sPath, bool bAllDirectories)

{

	string sTmp = sPath + string("\\*.*");
	WIN32_FIND_DATAA fd;

	HANDLE hFind = FindFirstFileA(sTmp.c_str(), &fd);

	if (INVALID_HANDLE_VALUE != hFind)

	{
		do
		{
			sTmp = sPath + string("\\") + string(fd.cFileName);

			if (sTmp.find(".jpg") != string::npos)
				vList.push_back(sTmp);

		} while (FindNextFileA(hFind, &fd));
		FindClose(hFind);

	}
}

BOOL CIMGViewerView::NextImgLoad(int m_nFileIdx)
{
	if (m_nFileIdx > -1 && m_nFileIdx<m_vtfilelist.size())
	{
		FILE* infile = NULL;
		if ((infile = fopen(m_vtfilelist.at(m_nFileIdx).c_str(), "rb")) != NULL)
		{
			if (m_hBitmap)
			{
				DeleteObject(m_hBitmap);
				m_hBitmap = NULL;
			}
			
			m_hBitmap = JpegStreamIn(infile);
			fclose(infile);
			USES_CONVERSION;
			AfxGetMainWnd()->SetWindowText(A2W(m_vtfilelist.at(m_nFileIdx).c_str()));
		
		}

		return TRUE;
	}
	else
	{
		
	}

	return FALSE;
}
BOOL CIMGViewerView::PreCreateWindow(CREATESTRUCT& cs)
{


	


	return CView::PreCreateWindow(cs);

}

// CIMGViewerView drawing

void CIMGViewerView::OnDraw(CDC* pDC)
{
	CIMGViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;


	if (!m_hBitmap)
		return;
	CRect rect;
	this->GetClientRect(&rect);
	CBitmap* bmp = CBitmap::FromHandle(m_hBitmap);
	BITMAP bm;
	bmp->GetObject(sizeof(BITMAP), &bm);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(bmp);
	pDC->SetStretchBltMode(HALFTONE);


	


	DWORD imgheight = rect.Height();
	DWORD  imgWidth = (bm.bmWidth* imgheight)/bm.bmHeight;

	DWORD x_pos = (rect.Width() / 2) - (imgWidth / 2);
	pDC->StretchBlt(x_pos, 0, imgWidth, imgheight, &memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	DeleteObject(memDC);
	DeleteObject(bmp);


}


// CIMGViewerView printing

BOOL CIMGViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CIMGViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing

}

void CIMGViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CIMGViewerView diagnostics

#ifdef _DEBUG
void CIMGViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CIMGViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CIMGViewerDoc* CIMGViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CIMGViewerDoc)));
	return (CIMGViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CIMGViewerView message handlers
HBITMAP CIMGViewerView::JpegStreamIn(FILE* infile)
{

	BITMAPINFO* pBmi = 0;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	UINT width = cinfo.output_width;
	UINT height = cinfo.output_height;

	DWORD dwSizeOfColorTable = (cinfo.out_color_components == 1) ? sizeof(RGBQUAD) * 256 : 0;
	DWORD dwSizeOfBitmapInfo = sizeof(BITMAPINFOHEADER) + dwSizeOfColorTable;

	pBmi = (BITMAPINFO*)(new BYTE[dwSizeOfBitmapInfo]);
	::ZeroMemory(pBmi, dwSizeOfBitmapInfo);

	// 일단 칼라 설정
	pBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmi->bmiHeader.biWidth = width;
	pBmi->bmiHeader.biHeight = height;
	pBmi->bmiHeader.biPlanes = 1;
	pBmi->bmiHeader.biBitCount = 24;
	m_pStatusBar->SetPaneText(0, _T("로드중."));
	CString str;
	str.Format(_T("%d X %d"), width, height);

	m_pStatusBar->SetPaneText(1, str);
	// 그레이스케일의 경우
	if (dwSizeOfColorTable){
		pBmi->bmiHeader.biBitCount = 8;
		// 256 그레이스케일의 색테이블값 설정
		for (int i = 0; i < 256; i++)
			pBmi->bmiColors[i].rgbBlue =
			pBmi->bmiColors[i].rgbGreen =
			pBmi->bmiColors[i].rgbRed = i;
	}

	//DIBSECTION 작성
	LPVOID pvBits;
	HBITMAP hBitmap = ::CreateDIBSection(0, pBmi, DIB_RGB_COLORS, &pvBits, 0, 0);
	if (!hBitmap) {
		if (pBmi)
			delete[]pBmi;
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}

	int row_stride = cinfo.output_width * cinfo.output_components;
	// 패딩 Width의 bytes
	DWORD widthBytes = WIDTHBYTES(width*pBmi->bmiHeader.biBitCount);

	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
	

	BYTE r, g, b;	
	int i = 0;
	switch (cinfo.out_color_components){
	case 1://GRAYSCALE
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			::CopyMemory((PBYTE)pvBits + widthBytes*(height - cinfo.output_scanline), buffer[0], width);
		}
		break;
	
	case 3://COLOR
		     // for문을 병렬로 수행할 때 사용할 Thread 갯수
		      // 

		while (cinfo.output_scanline < cinfo.output_height) {
			
			jpeg_read_scanlines(&cinfo, buffer, 1);
			PBYTE src = (PBYTE)buffer[0];
			PBYTE dst = (PBYTE)pvBits + widthBytes*(height - cinfo.output_scanline);
	  
			for (UINT j = 0; j < width; j++){
				r = *src++;  g = *src++; b = *src++;
				*dst++ = b;  *dst++ = g; *dst++ = r;

			
			}
		}
	}

	//AfxMessageBox(str);
	if (pBmi){
		delete[]pBmi;
		pBmi = 0;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	m_pStatusBar->SetPaneText(0, _T("로드완료"));
	return hBitmap;
}


BOOL CIMGViewerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (zDelta <0)
		NextImgLoad(++m_nFileIdx);
	else
		NextImgLoad(--m_nFileIdx);
	InvalidateRect(NULL);//화면 갱신
	
	
	// 상태바의 첫번째 판넬에 문자열 출력 

	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CIMGViewerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	m_pStatusBar = (CStatusBar *)(AfxGetMainWnd()->GetDlgItem(AFX_IDW_STATUS_BAR));
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	GetFiles(m_vtfilelist, ".\\", FALSE);
	USES_CONVERSION;
	if (m_vtfilelist.size() > 0)
	{
		m_nFileIdx++;
		NextImgLoad(m_nFileIdx);
		AfxGetMainWnd()->SetWindowText(A2W(m_vtfilelist.at(m_nFileIdx).c_str()));

	}
	else
	{
		AfxMessageBox(_T("There are no files;"));


	}
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
}
