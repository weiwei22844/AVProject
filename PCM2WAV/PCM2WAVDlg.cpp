// PCM2WAVDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCM2WAV.h"
#include "PCM2WAVDlg.h"
#include "Mmsystem.h"		// this header file is for PCM play(Winmm.lib)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

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
// CPCM2WAVDlg dialog

CPCM2WAVDlg::CPCM2WAVDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPCM2WAVDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPCM2WAVDlg)
	m_nChannel = 1;
	m_nSamplePSec = 8000;
	m_nBitPSample = 16;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_fileName = "";
	m_wavFlname = "";
	m_bWAVPlaying = FALSE;
	m_bPCMPlaying = FALSE;
}

void CPCM2WAVDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPCM2WAVDlg)
	DDX_Text(pDX, IDC_EDIT_CHANNEL, m_nChannel);
	DDX_Text(pDX, IDC_EDIT_SAMPPERSEC, m_nSamplePSec);
	DDX_Text(pDX, IDC_EDIT_BITPERSAMP, m_nBitPSample);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPCM2WAVDlg, CDialog)
	//{{AFX_MSG_MAP(CPCM2WAVDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_FILE, OnBtnFile)
	ON_BN_CLICKED(IDC_BTN_PLAYPCM, OnBtnPlaypcm)
	ON_BN_CLICKED(IDC_BTN_PALYWAV, OnBtnPalywav)
	ON_BN_CLICKED(IDC_BTN_WAVFILE, OnBtnWavfile)
	ON_WM_CTLCOLOR()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPCM2WAVDlg message handlers

BOOL CPCM2WAVDlg::OnInitDialog()
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
	
	// TODO: Add extra initialization here
	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPCM2WAVDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPCM2WAVDlg::OnPaint() 
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
HCURSOR CPCM2WAVDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPCM2WAVDlg::OnBtnFile() 
{
	// TODO: Add your control notification handler code here
    CString filter="PCM File (*.PCM)|*.PCM|ALL File (*.*)|*.*||";
    
    CFileDialog fileDlg(TRUE,NULL,NULL,NULL,filter,this);
    /////Specifies that the user can type only names of existing files in the File Name entry field. If this flag 
    ///is specified and the user enters an invalid name, the dialog box procedure displays a warning in a message box
    fileDlg.m_ofn.Flags|=OFN_FILEMUSTEXIST;
    fileDlg.m_ofn.lpstrTitle="Loading file...";
    if (fileDlg.DoModal()==IDOK) {
        AfxGetApp()->BeginWaitCursor();
        m_fileName = fileDlg.GetPathName();	///Returns the full path of the selected file
        SetDlgItemText(IDC_STATIC_FILE, m_fileName);		////file name text box
	}
}

BOOL CPCM2WAVDlg::TestInput()
{
	BOOL bRt;
	if(m_fileName.Compare("") == 0)
	{
		MessageBox("请选择要转换的PCM文件");
		return FALSE;
	}
	bRt = UpdateData(TRUE);
	if(!bRt)
	{
		return FALSE;
	}
	if(m_nChannel <= 0 || m_nChannel > 2)
	{
		MessageBox("声道数错误");
		return FALSE;
	}
	if(m_nSamplePSec <= 0)
	{
		MessageBox("采样频率必须为正数");
		return FALSE;
	}
	if(m_nBitPSample <= 0)
	{
		MessageBox("采样位数必须为正数");
		return FALSE;
	}

	return TRUE;
}

void CPCM2WAVDlg::OnOK() 
{
	// TODO: Add extra validation here
	BOOL bRt;
	bRt = TestInput();
	if(!bRt)
	{
		return;
	}

	HANDLE hFile = NULL;
	hFile = CreateFile(m_fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == NULL)
	{
		MessageBox("can not open source file");
		return;
	}	
	DWORD dwDatalen;
	dwDatalen = GetFileSize(hFile, NULL);
	if (dwDatalen == 0xFFFFFFFF)
	{
		CloseHandle(hFile);
		MessageBox("get data file length error");
		return;
	}
	BYTE* pBuf = new BYTE[dwDatalen];
	if(pBuf == NULL)
	{
		CloseHandle(hFile);
		MessageBox("Can not get enough memory");
		return;
	}
	
	DWORD len;
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	bRt = ReadFile(hFile, pBuf, dwDatalen, &len, NULL);
	if(!bRt)
	{
		MessageBox("Read file failed");
		CloseHandle(hFile);
		delete pBuf;
		return;
	}
	CloseHandle(hFile);
	
	WAVFILEHEADER wavfileHead;
    memcpy(wavfileHead.header, "RIFF", sizeof(wavfileHead.header));
	wavfileHead.dwSize = dwDatalen + sizeof(WAVFILEHEADER);
    memcpy(wavfileHead.wave, "WAVE", sizeof(wavfileHead.wave));
    memcpy(wavfileHead.Fmt, "fmt ", sizeof(wavfileHead.Fmt));
    wavfileHead.dwFmtSize = 16;
	wavfileHead.sWaveType = 1;

	wavfileHead.sChannelNum = m_nChannel;
	wavfileHead.dwSmpFrq = m_nSamplePSec;
	wavfileHead.dwSmpSzPerSnd = m_nBitPSample/8*m_nSamplePSec*m_nChannel;
	wavfileHead.sBypeOfSmp = m_nBitPSample/8*m_nChannel;
    wavfileHead.sBitOfSmp = m_nBitPSample;

	CHAR dataFlg[4];
	memcpy(dataFlg, "data", 4);

	FILE* pfile = fopen("Test.wav", "wb");
	if(pfile)
	{
		fwrite(&wavfileHead, 1, sizeof(WAVFILEHEADER), pfile);
		fwrite(dataFlg, 1, 4, pfile);
		fwrite(&dwDatalen, 1, 4, pfile);
		fwrite(pBuf, 1, dwDatalen, pfile);
		fclose(pfile);
		MessageBox("完成转换");
	}else{
		MessageBox("打开写入文件错误");
	}

	if(pBuf)
		delete pBuf;
}

UINT PlayPCM(void* pcmdata, int nLen, int nCha, int BitPSam, int SamPSec) 
{ 
	HWAVEOUT hwo; 
	WAVEHDR whdr; 
	MMRESULT mmres; 
	HANDLE hDoneEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("DONE_EVENT")); 
	UINT devId = 0; 
	WAVEFORMATEX wFormatEx; 
	
	wFormatEx.wFormatTag = WAVE_FORMAT_PCM; 
	wFormatEx.nChannels = nCha; 
	wFormatEx.wBitsPerSample = BitPSam; 
	wFormatEx.nSamplesPerSec = SamPSec; 
	wFormatEx.nBlockAlign = wFormatEx.wBitsPerSample * wFormatEx.nChannels / 8; 
	wFormatEx.nAvgBytesPerSec = wFormatEx.nSamplesPerSec * wFormatEx.nBlockAlign; 
	
	// Open audio device 
	for (devId = 0; devId < waveOutGetNumDevs(); devId++) { 
		mmres = waveOutOpen(&hwo, devId, &wFormatEx, (DWORD) hDoneEvent, 0, CALLBACK_EVENT); 
		if (mmres == MMSYSERR_NOERROR) { 
			break; 
		} 
	} 
	if (mmres != MMSYSERR_NOERROR) 
	{ 
		return mmres; 
	} 
	
	// Initialize wave header 
	ZeroMemory(&whdr, sizeof(WAVEHDR)); 
	whdr.lpData = (LPSTR)pcmdata; 
	whdr.dwBufferLength = nLen; 
	
	mmres = waveOutPrepareHeader(hwo, &whdr, sizeof(WAVEHDR)); 
	if (mmres != MMSYSERR_NOERROR) 
	{ 
		return mmres; 
	} 
	
	mmres = waveOutWrite(hwo, &whdr, sizeof(WAVEHDR)); 
	if (mmres != MMSYSERR_NOERROR) 
	{ 
		return mmres; 
	} 
	
	// Wait for audio to finish playing 
	while (!(whdr.dwFlags & WHDR_DONE)) 
	{ 
		WaitForSingleObject(hDoneEvent, INFINITE); 
	} 
	
	// Clean up 
	mmres = waveOutUnprepareHeader(hwo, &whdr, sizeof(WAVEHDR)); 
	if (mmres != MMSYSERR_NOERROR) { 
		return mmres; 
	} 
	
	mmres = waveOutClose(hwo); 
	if (mmres != MMSYSERR_NOERROR) 
	{ 
		return mmres; 
	} 
	
	return MMSYSERR_NOERROR; 
}

DWORD WINAPI PlayPcmFun(LPVOID pParam)
{
	CPCM2WAVDlg* pMainDlg = (CPCM2WAVDlg*)pParam;
	BOOL bRt;

	HANDLE hFile = NULL;
	hFile = CreateFile(pMainDlg->m_fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == NULL)
	{
		AfxMessageBox("can not open source file");
		return 0;
	}	
	DWORD dwDatalen;
	dwDatalen = GetFileSize(hFile, NULL);
	if (dwDatalen == 0xFFFFFFFF)
	{
		CloseHandle(hFile);
		AfxMessageBox("get data file length error");
		return 0;
	}
	BYTE* pBuf = new BYTE[dwDatalen];
	if(pBuf == NULL)
	{
		CloseHandle(hFile);
		AfxMessageBox("Can not get enough memory");
		return 0;
	}
	
	DWORD len;
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	bRt = ReadFile(hFile, pBuf, dwDatalen, &len, NULL);
	if(!bRt)
	{
		AfxMessageBox("Read file failed");
		CloseHandle(hFile);
		delete pBuf;
		return 0;
	}
	CloseHandle(hFile);
	
	PlayPCM(pBuf, dwDatalen, pMainDlg->m_nChannel, pMainDlg->m_nBitPSample, pMainDlg->m_nSamplePSec);
	
	if(pBuf)
		delete pBuf;

	AfxMessageBox("播放完毕！");
	pMainDlg->GetDlgItem(IDC_BTN_PLAYPCM)->EnableWindow(TRUE);

	return 1;
}

void CPCM2WAVDlg::OnBtnPlaypcm() 
{
	// TODO: Add your control notification handler code here
	BOOL bRt;
	bRt = TestInput();
	if(!bRt)
	{
		return;
	}

	HANDLE handl = CreateThread(NULL, 0, PlayPcmFun, this, 0, NULL);
	if(handl)
	{
		CloseHandle(handl);
		GetDlgItem(IDC_BTN_PLAYPCM)->EnableWindow(FALSE);
	}
}

DWORD WINAPI PlayWAV(void * param)
{
	CPCM2WAVDlg *pMaindDlg = (CPCM2WAVDlg *)param;
	LPSTR szFileName = pMaindDlg->m_fileName.GetBuffer(0);
	MMCKINFO mmckinfoParent;
	MMCKINFO mmckinfoSubChunk;
	DWORD dwFmtSize;
	HMMIO m_hmmio;//音频文件句柄
	DWORD m_WaveLong;
	HPSTR lpData;//音频数据
	HANDLE m_hFormat;
	WAVEFORMATEX * lpFormat;
	DWORD m_dwDataOffset;
	DWORD m_dwDataSize;
	WAVEHDR pWaveOutHdr;
	//WAVEOUTCAPS pwoc;
	HWAVEOUT hWaveOut;

	//打开波形文件
	if(!(m_hmmio = mmioOpen(szFileName,NULL,MMIO_READ|MMIO_ALLOCBUF)))
	{
		//File open Error
		MessageBox(NULL, "Failed to open the file.", "Error", MB_OK);
		return 0;
	}
	//检查打开文件是否是声音文件
	mmckinfoParent.fccType =mmioFOURCC('W', 'A', 'V', 'E');
	if(mmioDescend(m_hmmio, (LPMMCKINFO)&mmckinfoParent,NULL,MMIO_FINDRIFF))
	{
		//NOT WAVE FILE AND QUIT
		return 0;
	}
	//寻找 fmt 块
	mmckinfoSubChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if(mmioDescend(m_hmmio, &mmckinfoSubChunk, &mmckinfoParent, MMIO_FINDCHUNK))
	{
		//Can't find fmt chunk
	}
	//获得 fmt 块的大小，申请内存
	dwFmtSize = mmckinfoSubChunk.cksize ;
	m_hFormat = LocalAlloc(LMEM_MOVEABLE,LOWORD(dwFmtSize));
	if(!m_hFormat)
	{
		//failed alloc memory
		return 0;
	}
	lpFormat = (WAVEFORMATEX*)LocalLock(m_hFormat);
	if(!lpFormat)
	{
		//failed to lock the memory
	}
	if((unsigned long)mmioRead(m_hmmio,(HPSTR)lpFormat,dwFmtSize) != dwFmtSize)
	{
		//failed to read format chunk
	}
	//离开 fmt 块
	mmioAscend(m_hmmio, &mmckinfoSubChunk, 0);
	//寻找 data 块
	mmckinfoSubChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(mmioDescend(m_hmmio, &mmckinfoSubChunk, &mmckinfoParent, MMIO_FINDCHUNK))
	{
		//Can't find data chunk
		return 0;
	}
	//获得 data 块的大小
	m_dwDataSize = mmckinfoSubChunk.cksize;
	m_dwDataOffset = mmckinfoSubChunk.dwDataOffset;
	if(m_dwDataSize == 0L)
	{
		//no data in the data chunk
		return 0;
	}
	//为音频数据分配内存
	lpData = new char[m_dwDataSize];
	if(!lpData)
	{
		//faile
		return 0;
	}
	if(mmioSeek(m_hmmio, m_dwDataOffset, SEEK_SET) < 0)
	{
		//Failed to read the data chunk
		return FALSE;
	}
	m_WaveLong = mmioRead(m_hmmio, lpData, m_dwDataSize);
	if(m_WaveLong < 0)
	{
		//Failed to read the data chunk
	}
	//检查音频设备，返回音频输出设备的性能
	/*if(waveOutGetDeVCaps(WAVE_MAPPER, &pwoc, sizeof(WAVEOUTCAPS)) != 0)
	{
		//Unable to allocate or lock memory
	}*/
	//检查音频输出设备是否能播放指定的音频文件
	HANDLE hDoneEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("WAV_EVENT")); 
	if(waveOutOpen(&hWaveOut, WAVE_MAPPER, lpFormat, NULL, (DWORD)hDoneEvent, CALLBACK_NULL) != 0)
	{
		//Failed to OPEN the wave out devices
	}
	//准备待播放的数据
	pWaveOutHdr.lpData = (HPSTR)lpData;
	pWaveOutHdr.dwBufferLength = m_WaveLong;
	pWaveOutHdr.dwFlags = 0;
	if(waveOutPrepareHeader(hWaveOut, &pWaveOutHdr, sizeof(WAVEHDR)) != 0)
	{
		//Failed to prepare the wave data buffer
		return 0;
	}
	//播放音频数据文件
	if(waveOutWrite(hWaveOut, &pWaveOutHdr, sizeof(WAVEHDR)) != 0)
	{
		return 0;
	}
	while (!(pWaveOutHdr.dwFlags & WHDR_DONE)) 
	{ 
		WaitForSingleObject(hDoneEvent, INFINITE); 
	}

	//关闭音频输出设备,释放内存
	waveOutReset(hWaveOut);
	waveOutClose(hWaveOut);
	LocalUnlock(m_hFormat);
	LocalFree(m_hFormat);
	delete []lpData; 
	pMaindDlg->m_bWAVPlaying = FALSE;

	return 1;
}

void CPCM2WAVDlg::OnBtnPalywav() 
{
	// TODO: Add your control notification handler code here
	if(m_wavFlname.Compare("") == 0)
	{
		MessageBox("请选择要转换的WAV文件");
		return;
	}

#ifndef USE_WAVEOUT
	if(!m_bWAVPlaying)
	{
		SetDlgItemText(IDC_BTN_PALYWAV, "停止播放WAV");
		sndPlaySound(m_wavFlname.GetBuffer(0), SND_ASYNC);
		m_bWAVPlaying = TRUE;
	}else{
		SetDlgItemText(IDC_BTN_PALYWAV, "播放WAV");
		sndPlaySound(NULL, SND_ASYNC);
		m_bWAVPlaying = FALSE;
	}
#else
	if(m_bWAVPlaying)
	{
		return;
	}
	HANDLE handl = CreateThread(NULL, 0, PlayWAV, this, 0, NULL);
	if(handl)
	{
		m_bWAVPlaying = TRUE;
		CloseHandle(handl);
	}	
#endif

}

void CPCM2WAVDlg::OnBtnWavfile() 
{
	// TODO: Add your control notification handler code here
	CString filter="WAV File (*.WAV)|*.WAV|ALL File (*.*)|*.*||";
    
    CFileDialog fileDlg(TRUE,NULL,NULL,NULL,filter,this);
    /////Specifies that the user can type only names of existing files in the File Name entry field. If this flag 
    ///is specified and the user enters an invalid name, the dialog box procedure displays a warning in a message box
    fileDlg.m_ofn.Flags |= OFN_FILEMUSTEXIST;
    fileDlg.m_ofn.lpstrTitle = "Loading file...";
    if (fileDlg.DoModal() == IDOK) {
        AfxGetApp()->BeginWaitCursor();
        m_wavFlname = fileDlg.GetPathName();				///Returns the full path of the selected file
        SetDlgItemText(IDC_STATIC_WAVFILE, m_wavFlname);	////file name text box
	}
}

HBRUSH CPCM2WAVDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	HBRUSH hMyBr = NULL;
	switch(nCtlColor)
	{
		case CTLCOLOR_EDIT:
			pDC->SetTextColor(RGB(0, 0, 255));
			//pDC->SetBkColor(RGB(0,0,0));
			break;
		default:
			break;
	}

	switch(pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC_WAVFILE:
	case IDC_STATIC_FILE:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 0, 0));
		//pDC->SetBkColor(RGB(0,0,0));
		hMyBr = CreateSolidBrush(RGB(200, 200, 200)); //创建画刷
		break;
	default:
		break;
	}
		
	if(hMyBr)
	{
		return hMyBr; //返回画刷句柄
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CPCM2WAVDlg::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	UINT count; 
    char filePath[200]; 
    count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0); 
    if(count > 1) 
    {
        MessageBox("不支持对多个文件操作");
    }else if(count == 1)
    {
        int pathLen = DragQueryFile(hDropInfo, 0, filePath, sizeof(filePath));
        m_fileName = filePath;
        SetDlgItemText(IDC_STATIC_FILE, m_fileName);
		OnBtnPlaypcm();
	}

	CDialog::OnDropFiles(hDropInfo);
}
