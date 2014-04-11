// RecordAudioDlg.h : header file
//

#if !defined(AFX_RECORDAUDIODLG_H__E263D565_C893_4AA7_84AD_0578BC550AFD__INCLUDED_)
#define AFX_RECORDAUDIODLG_H__E263D565_C893_4AA7_84AD_0578BC550AFD__INCLUDED_

#include "AudioCtrl.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CRecordAudioDlg dialog

class CRecordAudioDlg : public CDialog
{
// Construction
public:
	CAudioCtrl m_AudioCtrl;
	CRecordAudioDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CRecordAudioDlg)
	enum { IDD = IDD_RECORDAUDIO_DIALOG };
	CSliderCtrl	m_PlaySndSlider;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordAudioDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CRecordAudioDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRecord();
	afx_msg void OnStop();
	afx_msg void OnSelchangeCmbInNames();
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnReleasedcaptureSliderRecord(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlay();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	afx_msg LRESULT OnMixerInfo(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	CString GetFileName(BOOL bOpen, LPCTSTR lpszTitle, LPCTSTR lpszFileName=NULL, char *szFilter=NULL);
	void SetRecordVolumeRand(DWORD dwMin,DWORD dwMax);
	void SetRecordDev(UINT uiIndex);
	void SetCurMixerIndex();
	void SetCurDevIndex(UINT uiIndex);
	void GetDevsName(char* pszName);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDAUDIODLG_H__E263D565_C893_4AA7_84AD_0578BC550AFD__INCLUDED_)
