// mfc_sampleDoc.h : interface of the CMfc_sampleDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MFC_SAMPLEDOC_H__D59A984D_017D_4B32_B8B9_0BAB54CA8296__INCLUDED_)
#define AFX_MFC_SAMPLEDOC_H__D59A984D_017D_4B32_B8B9_0BAB54CA8296__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMfc_sampleDoc : public CDocument
{
protected: // create from serialization only
	CMfc_sampleDoc();
	DECLARE_DYNCREATE(CMfc_sampleDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMfc_sampleDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMfc_sampleDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMfc_sampleDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFC_SAMPLEDOC_H__D59A984D_017D_4B32_B8B9_0BAB54CA8296__INCLUDED_)
