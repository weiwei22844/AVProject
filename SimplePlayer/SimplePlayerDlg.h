// SimplePlayerDlg.h : header file
//

#if !defined(AFX_SIMPLEPLAYERDLG_H__3599FE35_3322_4CC7_B30B_6D6050C2EDFF__INCLUDED_)
#define AFX_SIMPLEPLAYERDLG_H__3599FE35_3322_4CC7_B30B_6D6050C2EDFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerDlg dialog

#include <streams.h>
#include "CDXGraph.h"
#include "CTFullScreen.h"
#include "decode/interface.h"
#include "render/ddoffscreenrender.h"

#define SLIDER_TIMER   100
#define WM_CAPTURE_BITMAP   WM_APP + 1

class CSimplePlayerDlg : public CDialog
{
public:
    // how many times you've captured
    int m_nCapTimes;
// Construction
public:
	CSimplePlayerDlg(CWnd* pParent = NULL);	// standard constructor
	~CSimplePlayerDlg();
    static UINT WINAPI DecodeThread(LPVOID param);
    LONG OnDisplayFrameMsg(WPARAM wParam, LPARAM lParam);

    IDecode* m_Decoder;
    DDOffscreenRender m_OffscrnRender;
    BOOL m_bPlaying;
    BOOL m_bPause;
    int m_nWidth;
    int m_nHeight;

// Dialog Data
	//{{AFX_DATA(CSimplePlayerDlg)
	enum { IDD = IDD_SIMPLEPLAYER_DIALOG };
	CSliderCtrl	mSliderGraph;
	CStatic	mVideoWindow;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimplePlayerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CDXGraph *   mFilterGraph;     // Filter Graph封装
	CString      mSourceFile;      // 源文件
	UINT         mSliderTimer;     // 定时器ID

    CTFullScreen	m_FullScreenWnd;        // added by ZWW for testing
    HANDLE  m_hThread;
    BOOL m_bThdRun;

	void CreateGraph(void);        // 创建Filter Graph
	void DestroyGraph(void);       // 析构Filter Graph
	void RestoreFromFullScreen(void);

	// Just for testing...
	HRESULT FindFilterByInterface(REFIID riid, IBaseFilter** ppFilter);
	void ShowVRPropertyPage(void);

	// Generated message map functions
	//{{AFX_MSG(CSimplePlayerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonOpen();
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonPause();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonGrab();
	afx_msg void OnButtonFullscreen();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonTest();
	afx_msg void OnBtnRatedown();
	afx_msg void OnBtnRateup();
	afx_msg void OnBtnRatenormal();
	//}}AFX_MSG
	afx_msg LRESULT OnGraphNotify(WPARAM inWParam, LPARAM inLParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLEPLAYERDLG_H__3599FE35_3322_4CC7_B30B_6D6050C2EDFF__INCLUDED_)
