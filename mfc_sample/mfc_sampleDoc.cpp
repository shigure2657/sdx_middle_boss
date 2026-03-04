// mfc_sampleDoc.cpp : implementation of the CMfc_sampleDoc class
//

#include "stdafx.h"
#include "mfc_sample.h"

#include "mfc_sampleDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDoc

IMPLEMENT_DYNCREATE(CMfc_sampleDoc, CDocument)

BEGIN_MESSAGE_MAP(CMfc_sampleDoc, CDocument)
	//{{AFX_MSG_MAP(CMfc_sampleDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDoc construction/destruction

CMfc_sampleDoc::CMfc_sampleDoc()
{
	// TODO: add one-time construction code here

}

CMfc_sampleDoc::~CMfc_sampleDoc()
{
}

BOOL CMfc_sampleDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDoc serialization

void CMfc_sampleDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDoc diagnostics

#ifdef _DEBUG
void CMfc_sampleDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMfc_sampleDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDoc commands
