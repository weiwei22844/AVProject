// RecordAudioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RecordAudio.h"
#include "RecordAudioDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
#define WM_MAPER_NOTIFY  WM_USER+1001

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordAudioDlg dialog

CRecordAudioDlg::CRecordAudioDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecordAudioDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRecordAudioDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRecordAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordAudioDlg)
	DDX_Control(pDX, IDC_SLIDER_PLAY, m_PlaySndSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRecordAudioDlg, CDialog)
	//{{AFX_MSG_MAP(CRecordAudioDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnRecord)
	ON_BN_CLICKED(IDCANCEL, OnStop)
	ON_CBN_SELCHANGE(IDC_CMB_IN_NAMES, OnSelchangeCmbInNames)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_RECORD, OnReleasedcaptureSliderRecord)
	ON_BN_CLICKED(IDC_BUTTON2, OnPlay)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MAPER_NOTIFY, OnMixerInfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordAudioDlg message handlers

BOOL CALLBACK InputLineProc (UINT uLineIndex, MIXERLINE* pLineInfo, DWORD dwUserValue)
{
	::SendMessage((HWND)dwUserValue, WM_MAPER_NOTIFY, (WPARAM)uLineIndex, (LPARAM)pLineInfo->szName);
	return TRUE;
}

BOOL CRecordAudioDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	char szDevName[100] = {0};
	UINT uiDevs = m_AudioCtrl.GetWaveInDevs();
    UINT i;
	for(i=0; i<uiDevs; i++)
	{
		strcpy(szDevName, m_AudioCtrl.GetWaveInName(i));
		GetDevsName(szDevName);
	}

    uiDevs = waveOutGetNumDevs();
    for(i = 0; i<uiDevs; i++)
    {
        WAVEOUTCAPS tagCaps;
        MMRESULT rs = waveOutGetDevCaps(i, &tagCaps, sizeof(tagCaps));
        if (rs == MMSYSERR_NOERROR)
        {
            ((CComboBox*)GetDlgItem(IDC_CMB_DEVS_PLAY))->AddString(tagCaps.szPname);
            ((CComboBox*)GetDlgItem(IDC_CMB_DEVS_PLAY))->SetCurSel(0);
	    }
    }
	
	UINT uiAvailabeDev = m_AudioCtrl.GetWaveInAvailableDev();
	SetCurDevIndex(uiAvailabeDev);

	CVolumeCtrl::EnumerateInputLines(uiAvailabeDev, InputLineProc, (DWORD)this->m_hWnd);
	SetCurMixerIndex();

	DWORD dwMinVolume = 0, dwMaxVolume = 0;
	DWORD dwCurVolume = m_AudioCtrl.SetMixerDevs(0, &dwMinVolume, &dwMaxVolume);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_RECORD))->SetRange(dwMinVolume, dwMaxVolume);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_RECORD))->SetPos(dwCurVolume);

    m_PlaySndSlider.SetRange(0, 100);
    m_PlaySndSlider.SetPos(50);

	CRect rc;
	GetDlgItem(IDC_SATC_GRAPH)->GetWindowRect(&rc);
	ScreenToClient(&rc);

	if (!m_AudioCtrl.InitCtrl(((CComboBox*)GetDlgItem(IDC_CMB_DEVS_NAME))->GetCurSel(), 1024*4, GetSafeHwnd(), rc, NULL, (WPARAM)this))
	{
		MessageBox ( "Initialize callback device failed" );
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

LRESULT CRecordAudioDlg::OnMixerInfo(WPARAM wParam, LPARAM lParam)
{
	CComboBox* pCmb = (CComboBox*)GetDlgItem(IDC_CMB_IN_NAMES);
	if (pCmb)
	{
		pCmb->AddString(LPCTSTR(lParam));
	}

	return S_OK;
}

void CRecordAudioDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRecordAudioDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRecordAudioDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CRecordAudioDlg::GetDevsName(char *pszName)
{
	CComboBox* pCmbox = (CComboBox*)GetDlgItem(IDC_CMB_DEVS_NAME);
	if (pCmbox)
	{
		pCmbox->AddString(pszName);
	}
}

void CRecordAudioDlg::SetCurDevIndex(UINT uiIndex)
{
	CComboBox* pCmbox = (CComboBox*)GetDlgItem(IDC_CMB_DEVS_NAME);
	if (pCmbox)
	{
		pCmbox->SetCurSel(uiIndex);
	}
}

void CRecordAudioDlg::SetCurMixerIndex()
{
	CComboBox* pCmbox = (CComboBox*)GetDlgItem(IDC_CMB_IN_NAMES);
	if (pCmbox)
	{
		pCmbox->SetCurSel(0);
	}
}

void CRecordAudioDlg::OnRecord() 
{
    // added by ZWW
    m_AudioCtrl.SetDeviceID(((CComboBox*)GetDlgItem(IDC_CMB_DEVS_NAME))->GetCurSel());

	CComboBox* pCmb = (CComboBox*)GetDlgItem(IDC_CMB_IN_NAMES);

	m_AudioCtrl.Record(pCmb->GetCurSel(), 11025*2, 16);
	m_AudioCtrl.SetRecordAudioFile(0, "text.wav");
}

void CRecordAudioDlg::OnStop() 
{
	m_AudioCtrl.Stop();	
}

void CRecordAudioDlg::SetRecordDev(UINT uiIndex)
{
	m_AudioCtrl.SetMixerDevs(uiIndex);
}

void CRecordAudioDlg::OnSelchangeCmbInNames() 
{
	m_AudioCtrl.SetMixerDevs(((CComboBox*)GetDlgItem(IDC_CMB_IN_NAMES))->GetCurSel());	
}

void CRecordAudioDlg::OnOK() 
{
	CDialog::OnOK();
}

void CRecordAudioDlg::OnClose() 
{
	OnOK();
}

void CRecordAudioDlg::SetRecordVolumeRand(DWORD dwMin, DWORD dwMax)
{

}

void CRecordAudioDlg::OnReleasedcaptureSliderRecord(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CSliderCtrl* pRecord = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_RECORD);
	if (pRecord)
	{
		m_AudioCtrl.SetCurVolume(pRecord->GetPos());
	}
	*pResult = 0;
}

void CRecordAudioDlg::OnPlay() 
{
	if (!m_AudioCtrl.IsPlay())
	{
		CString csFileName = GetFileName ( TRUE, "打开文件来播放", NULL, "Wave files(*.wav)|*.wav||" );
		if (csFileName.IsEmpty())
		{
			return;
		}

		m_AudioCtrl.SetDeviceID(((CComboBox*)GetDlgItem(IDC_CMB_DEVS_PLAY))->GetCurSel());
		m_AudioCtrl.Play(csFileName.GetBuffer(csFileName.GetLength()));
        int nPos = m_PlaySndSlider.GetPos();
        DWORD dwVolume = (0xFFFF*nPos/100);
        dwVolume |= (dwVolume<<16);
        m_AudioCtrl.SetPlayVolume(dwVolume);
    }else{
        MessageBox("正在播放");
    }
}

CString CRecordAudioDlg::GetFileName(BOOL bOpen, LPCTSTR lpszTitle, LPCTSTR lpszFileName, char *szFilter)
{

	char szCurDir[MAX_PATH] = {0};
	DWORD dwRet = ::GetModuleFileName(NULL,(LPTSTR)szCurDir,sizeof(szCurDir));
	if ( szCurDir[dwRet-1] != '\\' )
		strncat ( szCurDir, "\\", sizeof(szCurDir) );

	CFileDialog FileDlg ( bOpen, ".wav", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL );
	FileDlg.m_ofn.lpstrTitle = LPSTR ( lpszTitle );
	if ( FileDlg.DoModal () != IDOK )
		return "";
	return FileDlg.GetPathName();
}

void CRecordAudioDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
    if(nSBCode == SB_THUMBTRACK)
    {
	    if(pScrollBar->m_hWnd == m_PlaySndSlider.m_hWnd)
        {
            DWORD dwVolume = (0xFFFF*nPos/100);
            dwVolume |= (dwVolume<<16);
            m_AudioCtrl.SetPlayVolume(dwVolume);
        }
    }
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
