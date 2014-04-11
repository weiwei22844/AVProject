// PCM2WAVDlg.h : header file
//

#if !defined(AFX_PCM2WAVDLG_H__FE37E724_5A98_4149_A1FB_AEDBD4B02F0C__INCLUDED_)
#define AFX_PCM2WAVDLG_H__FE37E724_5A98_4149_A1FB_AEDBD4B02F0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _WAVFILEHEADER{
    CHAR header[4];
    DWORD dwSize;
    CHAR wave[4];
    CHAR Fmt[4];
    DWORD dwFmtSize;
    SHORT sWaveType;
    SHORT sChannelNum;
    DWORD dwSmpFrq;
    DWORD dwSmpSzPerSnd;
    SHORT sBypeOfSmp;
    SHORT sBitOfSmp;
}WAVFILEHEADER;
/////////////////////////////////////////////////////////////////////////////
// CPCM2WAVDlg dialog

class CPCM2WAVDlg : public CDialog
{
// Construction
public:
	CPCM2WAVDlg(CWnd* pParent = NULL);	// standard constructor

public:
	CString m_fileName;
	CString m_wavFlname;
	BOOL m_bWAVPlaying;
	BOOL m_bPCMPlaying;

	BOOL TestInput();
// Dialog Data
	//{{AFX_DATA(CPCM2WAVDlg)
	enum { IDD = IDD_PCM2WAV_DIALOG };
	int		m_nChannel;
	int		m_nSamplePSec;
	int		m_nBitPSample;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPCM2WAVDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPCM2WAVDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnFile();
	virtual void OnOK();
	afx_msg void OnBtnPlaypcm();
	afx_msg void OnBtnPalywav();
	afx_msg void OnBtnWavfile();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PCM2WAVDLG_H__FE37E724_5A98_4149_A1FB_AEDBD4B02F0C__INCLUDED_)
