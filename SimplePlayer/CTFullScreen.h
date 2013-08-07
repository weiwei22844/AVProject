#if !defined(AFX_CTFULLSCREEN_H__2D1F1133_8222_45C9_A4CC_3AFD372507C5__INCLUDED_)
#define AFX_CTFULLSCREEN_H__2D1F1133_8222_45C9_A4CC_3AFD372507C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CTFullScreen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTFullScreen window

class CTFullScreen : public CWnd
{
// Construction
public:
	CTFullScreen();

// Attributes
public:

// Operations
public:
	BOOL Create(HWND hParentWnd,HWND hMsgDrainWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTFullScreen)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTFullScreen();

	// Generated message map functions
protected:
	HWND		m_hParentWnd;
	HWND		m_hMsgDrainWnd;
	//{{AFX_MSG(CTFullScreen)
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTFULLSCREEN_H__2D1F1133_8222_45C9_A4CC_3AFD372507C5__INCLUDED_)
