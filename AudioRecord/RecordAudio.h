// RecordAudio.h : main header file for the RECORDAUDIO application
//

#if !defined(AFX_RECORDAUDIO_H__E7555F67_4347_40C2_A2E7_AE83A5D39EFF__INCLUDED_)
#define AFX_RECORDAUDIO_H__E7555F67_4347_40C2_A2E7_AE83A5D39EFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRecordAudioApp:
// See RecordAudio.cpp for the implementation of this class
//

class CRecordAudioApp : public CWinApp
{
public:
	CRecordAudioApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordAudioApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRecordAudioApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDAUDIO_H__E7555F67_4347_40C2_A2E7_AE83A5D39EFF__INCLUDED_)
