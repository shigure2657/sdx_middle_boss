// mfc_sampleView.h : interface of the CMfc_sampleView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MFC_SAMPLEVIEW_H__B66A9C05_96AA_485D_8F6A_9753C3D8E960__INCLUDED_)
#define AFX_MFC_SAMPLEVIEW_H__B66A9C05_96AA_485D_8F6A_9753C3D8E960__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMfc_sampleView : public CView
{
protected: // create from serialization only
	CMfc_sampleView();
	DECLARE_DYNCREATE(CMfc_sampleView)

// Attributes
public:
	CMfc_sampleDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMfc_sampleView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMfc_sampleView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMfc_sampleView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in mfc_sampleView.cpp
inline CMfc_sampleDoc* CMfc_sampleView::GetDocument()
   { return (CMfc_sampleDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFC_SAMPLEVIEW_H__B66A9C05_96AA_485D_8F6A_9753C3D8E960__INCLUDED_)
