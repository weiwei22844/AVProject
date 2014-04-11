// PCM2WAV.h : main header file for the PCM2WAV application
//

#if !defined(AFX_PCM2WAV_H__044DD991_B43E_404D_B9CC_038CA014779E__INCLUDED_)
#define AFX_PCM2WAV_H__044DD991_B43E_404D_B9CC_038CA014779E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPCM2WAVApp:
// See PCM2WAV.cpp for the implementation of this class
//

class CPCM2WAVApp : public CWinApp
{
public:
	CPCM2WAVApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPCM2WAVApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPCM2WAVApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PCM2WAV_H__044DD991_B43E_404D_B9CC_038CA014779E__INCLUDED_)
