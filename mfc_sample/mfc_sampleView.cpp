// mfc_sampleView.cpp : implementation of the CMfc_sampleView class
//

#include "stdafx.h"
#include "mfc_sample.h"

#include "mfc_sampleDoc.h"
#include "mfc_sampleView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleView

IMPLEMENT_DYNCREATE(CMfc_sampleView, CView)

BEGIN_MESSAGE_MAP(CMfc_sampleView, CView)
	//{{AFX_MSG_MAP(CMfc_sampleView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleView construction/destruction

CMfc_sampleView::CMfc_sampleView()
{
	// TODO: add construction code here

}

CMfc_sampleView::~CMfc_sampleView()
{
}

BOOL CMfc_sampleView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleView drawing

void CMfc_sampleView::OnDraw(CDC* pDC)
{
	CMfc_sampleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleView printing

BOOL CMfc_sampleView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMfc_sampleView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMfc_sampleView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleView diagnostics

#ifdef _DEBUG
void CMfc_sampleView::AssertValid() const
{
	CView::AssertValid();
}

void CMfc_sampleView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMfc_sampleDoc* CMfc_sampleView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMfc_sampleDoc)));
	return (CMfc_sampleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleView message handlers
