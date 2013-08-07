// SimplePlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SimplePlayer.h"
#include "SimplePlayerDlg.h"
#include "TList.h"
#include <process.h>
 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RCWIDTH(rc)					(rc.right - rc.left)
#define RCHEIGHT(rc)				(rc.bottom - rc.top)
extern CALLBACKINFO cb;
extern BOOL g_bOneShot;
extern HWND g_hwnd;
extern CTList m_list;
/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerDlg dialog

CSimplePlayerDlg::CSimplePlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSimplePlayerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSimplePlayerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	mFilterGraph = NULL;
	mSourceFile  = "";
	mSliderTimer = 0;
    m_hThread = NULL;
    m_bThdRun = FALSE;
}

CSimplePlayerDlg::~CSimplePlayerDlg()
{
	DestroyGraph();
}

void CSimplePlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSimplePlayerDlg)
	DDX_Control(pDX, IDC_SLIDER_GRAPH, mSliderGraph);
	DDX_Control(pDX, IDC_VIDEO_WINDOW, mVideoWindow);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSimplePlayerDlg, CDialog)
	//{{AFX_MSG_MAP(CSimplePlayerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_GRAB, OnButtonGrab)
	ON_BN_CLICKED(IDC_BUTTON_FULLSCREEN, OnButtonFullscreen)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_TEST, OnButtonTest)
	ON_BN_CLICKED(IDC_BTN_RATEDOWN, OnBtnRatedown)
	ON_BN_CLICKED(IDC_BTN_RATEUP, OnBtnRateup)
	ON_BN_CLICKED(IDC_BTN_RATENORMAL, OnBtnRatenormal)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerDlg message handlers

BOOL CSimplePlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	// Change the window style for video window, or video window
	// will not be painted properly.
	mVideoWindow.ModifyStyle(0, WS_CLIPCHILDREN);

	mSliderGraph.SetRange(0, 1000);
	mSliderGraph.SetPos(0);

    m_nCapTimes = 0;
    mCB.pOwner = this;
    g_hwnd = GetSafeHwnd();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSimplePlayerDlg::OnPaint() 
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
HCURSOR CSimplePlayerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSimplePlayerDlg::OnButtonOpen() 
{
	// TODO: Add your control notification handler code here
	CString    strFilter = "AVI File (*.avi)|*.avi|";
	strFilter += "MPEG File (*.mpg;*.mpeg)|*.mpg;*.mpeg|";
	strFilter += "Mp3 File (*.mp3)|*.mp3|";
	strFilter += "Wave File (*.wav)|*.wav|";
	strFilter += "All Files (*.*)|*.*|";
	CFileDialog dlgOpen(TRUE, NULL, NULL, OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, 
		strFilter, this);
	if (IDOK == dlgOpen.DoModal()) 
	{
		mSourceFile = dlgOpen.GetPathName();
		// Rebuild the file playback filter graph
		CreateGraph();
	}
}

UINT WINAPI CSimplePlayerDlg::DecodeThread(LPVOID param)
{
    TCHAR m_ShortName[MAX_PATH];
    int i = 0;
    wsprintf( m_ShortName, TEXT("snap%4.4ld.bmp\0"), i);
    pNode pnode = NULL;
    HWND hwnd = (HWND)param;
    char temp[30] = {0};
    HANDLE hf = NULL;
    
    while(TRUE)
    {
        m_list.getNode(&pnode);
        if(!pnode)
        {
            continue;
        }
        hf = CreateFile(m_ShortName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, NULL, NULL );
        if( hf == INVALID_HANDLE_VALUE )
            return 0;
        
        // write out the file header
        //
        BITMAPFILEHEADER bfh;
        memset( &bfh, 0, sizeof( bfh ) );
        bfh.bfType = 'MB';
        bfh.bfSize = sizeof( bfh ) + pnode->lSize + sizeof( BITMAPINFOHEADER );
        bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );
        
        DWORD dwWritten = 0;
        WriteFile( hf, &bfh, sizeof( bfh ), &dwWritten, NULL );
        
        // and the bitmap format
        //
        BITMAPINFOHEADER bih;
        memset( &bih, 0, sizeof( bih ) );
        bih.biSize = sizeof( bih );
        bih.biWidth = mCB.lWidth;
        bih.biHeight = mCB.lHeight;
        bih.biPlanes = 1;
        bih.biBitCount = 24;
        
        dwWritten = 0;
        WriteFile( hf, &bih, sizeof( bih ), &dwWritten, NULL );
        
        // and the bits themselves
        //
        dwWritten = 0;
        WriteFile( hf, pnode->pbBuffer, pnode->lSize, &dwWritten, NULL );
        CloseHandle( hf );
        if(pnode->pbBuffer)
        {
            delete pnode->pbBuffer;
            pnode->pbBuffer = NULL;
        }
        delete pnode;
        pnode = NULL;
        i++;
        wsprintf( m_ShortName, TEXT("snap%4.4ld.bmp\0"), i);
    }
    
    return 1;
}

void CSimplePlayerDlg::OnButtonPlay() 
{
	if (mFilterGraph)
	{
        CString CapDir;
        TCHAR szFilename[MAX_PATH], szFile[MAX_PATH];
        wsprintf( szFilename, TEXT("%sStillCap%04d.avi\0"), (LPCTSTR) CapDir, m_nCapTimes );
        wsprintf( szFile, TEXT("StillCap%04d.avi\0"), m_nCapTimes );
        _tcsncpy( mCB.m_szCapDir, CapDir, NUMELMS(mCB.m_szCapDir) );
        g_bOneShot = TRUE;

        m_hThread = (HANDLE)_beginthreadex(NULL, 0, DecodeThread, m_hWnd, 0, NULL);
        if (m_hThread == NULL) {
            MessageBox(_T("StartProcess failed"), NULL, MB_OK);
	    }

		mFilterGraph->Run();
		// Start a timer
		if (mSliderTimer == 0)
		{
			mSliderTimer = SetTimer(SLIDER_TIMER, 100, NULL);
		}
	}
}

void CSimplePlayerDlg::OnButtonPause() 
{
	if (mFilterGraph)
	{
		mFilterGraph->Pause();
		// Start a timer
		if (mSliderTimer == 0)
		{
			mSliderTimer = SetTimer(SLIDER_TIMER, 100, NULL);
		}
	}
}

void CSimplePlayerDlg::OnButtonStop() 
{
	if (mFilterGraph)
	{
		mFilterGraph->SetCurrentPosition(0);
		mFilterGraph->Stop();
		// Stop the timer
		if (mSliderTimer)
		{
			KillTimer(mSliderTimer);
			mSliderTimer = 0;
		}
	}
}

void CSimplePlayerDlg::OnButtonGrab() 
{
	if (mFilterGraph)
	{
		// Firstly grab a bitmap to a temp file
		char  szTemp[] = "D:\\mysnapshot.bmp";
		if (mFilterGraph->SnapshotBitmap(szTemp))
		{
			// User can browser for a new file here
			CString   strFilter = "BMP File (*.bmp)|*.bmp|";
			CFileDialog dlgOpen(FALSE, ".bmp", NULL, OFN_HIDEREADONLY, strFilter, NULL);
			if (IDOK == dlgOpen.DoModal()) 
			{
				::CopyFile(szTemp, dlgOpen.GetPathName(), FALSE);
				::DeleteFile(szTemp);
			}
		}
	}
}

void CSimplePlayerDlg::OnButtonFullscreen() 
{
	/*if (mFilterGraph)
	{
		mFilterGraph->SetFullScreen(TRUE);
	}*/

    // modified by ZWW for testing
    if (mFilterGraph)
    {
        int cx = GetSystemMetrics(SM_CXSCREEN);
        int cy = GetSystemMetrics(SM_CYSCREEN);
        
        CRect rect(0, 0, cx, cy);
        if (m_FullScreenWnd.Create(m_hWnd, NULL))
        {
            m_FullScreenWnd.SetWindowText(_T("{FULLSCREENWND}"));
            
            WINDOWPLACEMENT WndPlacement;
            mVideoWindow.SetParent(&m_FullScreenWnd);	
            m_FullScreenWnd.SetFocus();
            mVideoWindow.GetWindowPlacement(&WndPlacement);
            WndPlacement.showCmd = SW_SHOWMAXIMIZED;
            mVideoWindow.SetWindowPlacement(&WndPlacement);
            mFilterGraph->ResizeVideoWindow(0, 0, cx, cy);
            //CWnd::SetWindowPos(&wndTopMost, 0, 0, cx, cy, SWP_SHOWWINDOW);
        }
	}
}

void CSimplePlayerDlg::CreateGraph(void)
{
	DestroyGraph();

	mFilterGraph = new CDXGraph();
	if (mFilterGraph->Create())
	{
		// Render the source clip
		mFilterGraph->RenderFile(mSourceFile);
		// Set video window and notification window
		mFilterGraph->SetDisplayWindow(mVideoWindow.GetSafeHwnd());
		mFilterGraph->SetNotifyWindow(this->GetSafeHwnd());
		// Show the first frame
		mFilterGraph->Pause();
	}
}

void CSimplePlayerDlg::DestroyGraph(void)
{
	if (mFilterGraph)
	{
		// Stop the filter graph first
		mFilterGraph->Stop();
		mFilterGraph->SetNotifyWindow(NULL);

		delete mFilterGraph;
		mFilterGraph = NULL;
	}
}

BOOL CSimplePlayerDlg::OnEraseBkgnd(CDC* pDC)
{
	// Intercept background erasing for the movie window, since the
    // video renderer will keep the screen painted.  Without this code,
    // your video window might get painted over with gray (the default
    // background brush) when it is obscured by another window and redrawn.
    CRect rc;
    // Get the bounding rectangle for the movie screen
    mVideoWindow.GetWindowRect(&rc);
    ScreenToClient(&rc);
    // Exclude the clipping region occupied by our movie screen
    pDC->ExcludeClipRect(&rc);

	// Erase the remainder of the dialog as usual
    return CDialog::OnEraseBkgnd(pDC);
}

// We use "Return" key and "Esc" key to restore from fullscreen mode
BOOL CSimplePlayerDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			// Restore form fullscreen mode
			RestoreFromFullScreen();
			return 1;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CSimplePlayerDlg::RestoreFromFullScreen(void)
{
    mVideoWindow.SetParent(this);
    if(m_FullScreenWnd)
    {
        m_FullScreenWnd.DestroyWindow();
    }
    
    SetFocus();
    RECT rc;
    mVideoWindow.GetClientRect(&rc);
    LONG ht = RCHEIGHT(rc);
    LONG wt = RCWIDTH(rc);
    mFilterGraph->ResizeVideoWindow(rc.left, rc.top, wt, ht);
    InvalidateRect(NULL);       // added by ZWW for testing
}

void CSimplePlayerDlg::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == mSliderTimer && mFilterGraph)
	{
		double pos = 0, duration = 1.;
		mFilterGraph->GetCurrentPosition(&pos);
		mFilterGraph->GetDuration(&duration);
		// Get the new position, and update the slider
		int newPos = int(pos * 1000 / duration);
		if (mSliderGraph.GetPos() != newPos)
		{
			mSliderGraph.SetPos(newPos);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL CSimplePlayerDlg::DestroyWindow() 
{
	if (mSliderTimer)
	{
		KillTimer(mSliderTimer);
		mSliderTimer = 0;
	}
	
	return CDialog::DestroyWindow();
}

void CSimplePlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar->GetSafeHwnd() == mSliderGraph.GetSafeHwnd())
	{
		if (mFilterGraph)
		{
			// Calculate the current seeking position
			double duration = 1.;
			mFilterGraph->GetDuration(&duration);
			double pos = duration * mSliderGraph.GetPos() / 1000.;
			mFilterGraph->SetCurrentPosition(pos);
		}		
	}
	else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}

// Deal with the graph events 
LRESULT CSimplePlayerDlg::OnGraphNotify(WPARAM inWParam, LPARAM inLParam)
{
	IMediaEventEx * pEvent = NULL;
	if (mFilterGraph && (pEvent = mFilterGraph->GetEventHandle()))
	{
		LONG   eventCode = 0, eventParam1 = 0, eventParam2 = 0;
        //double duration = 1.;
		while (SUCCEEDED(pEvent->GetEvent(&eventCode, &eventParam1, &eventParam2, 0)))
		{ 
			// Spin through the events
			pEvent->FreeEventParams(eventCode, eventParam1, eventParam2);
			switch (eventCode)
			{
			case EC_COMPLETE:
		        //mFilterGraph->GetDuration(&duration);
                //mFilterGraph->SetCurrentPosition(duration);
				OnButtonPause();
				mFilterGraph->SetCurrentPosition(0);
				break;
			
			case EC_USERABORT:
			case EC_ERRORABORT:
				OnButtonStop();
				break;

			default:
				break;
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Locate a filter within the graph by searching (from renderers upstream)
// looking for a specific interface on the filter
HRESULT CSimplePlayerDlg::FindFilterByInterface(REFIID riid, IBaseFilter** ppFilter)
{
    *ppFilter = NULL;
	if (!mFilterGraph)
	{
		return E_FAIL;
	}

    IEnumFilters* pEnum;
    HRESULT hr = mFilterGraph->GetGraph()->EnumFilters(&pEnum);
    if (FAILED(hr)) 
	{
		return hr;
    }

    IBaseFilter* pFilter = NULL;
    while (pEnum->Next(1, &pFilter, NULL) == S_OK) 
	{
		// Check for required interface
		IUnknown* pUnk;
		HRESULT hrQuery = pFilter->QueryInterface(riid, (void**)&pUnk);
		if (SUCCEEDED(hrQuery)) 
		{
			pUnk->Release();
			pEnum->Release();
			*ppFilter = pFilter;
			return S_OK;
		}
		pFilter->Release();
    }
    pEnum->Release();

    return E_FAIL;
}

void CSimplePlayerDlg::ShowVRPropertyPage(void)
{
	IBaseFilter *pFilter = NULL;
	if (FAILED(FindFilterByInterface(IID_IVideoWindow, &pFilter)))
	{
		return;
	}
	pFilter->Release();

	ISpecifyPropertyPages *pProp = NULL;
	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr)) 
	{
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo); 
		IUnknown *pFilterUnk;
		pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

		// Show the page. 
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		pProp->Release();
		OleCreatePropertyFrame(
			this->GetSafeHwnd(),                   // Parent window
			0, 0,                   // Reserved
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			&pFilterUnk,            // Array of object pointers. 
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL                 // Reserved
		);

		// Clean up.
		pFilterUnk->Release();
		FilterInfo.pGraph->Release(); 
		CoTaskMemFree(caGUID.pElems);
	}
}

void CSimplePlayerDlg::OnButtonTest() 
{
	ShowVRPropertyPage();
}

void CSimplePlayerDlg::OnBtnRatedown() 
{
	// TODO: Add your control notification handler code here
    if (!mFilterGraph)
    {
        return;
    }
    
    if(!mFilterGraph->ModifyRate(-0.1))
    {
        MessageBox(_T("ModifyRate failed"));
        return;
    }
}

void CSimplePlayerDlg::OnBtnRateup() 
{
	// TODO: Add your control notification handler code here
    if (!mFilterGraph)
    {
        return;
    }
    
    if(!mFilterGraph->ModifyRate(0.1))
    {
        MessageBox(_T("ModifyRate failed"));
        return;
    }
}

void CSimplePlayerDlg::OnBtnRatenormal() 
{
	// TODO: Add your control notification handler code here
    if (!mFilterGraph)
    {
        return;
    }
    
    if(!mFilterGraph->SetRate(1.0))
    {
        MessageBox(_T("Set normal rate failed"));
        return;
    }
}

LRESULT CSimplePlayerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
    if (message == WM_CAPTURE_BITMAP)
        OutputDebugString(_T("WM_CAPTURE_BITMAP message"));//mCB.CopyBitmap(cb.dblSampleTime, cb.pBuffer, cb.lBufferSize);
	return CDialog::WindowProc(message, wParam, lParam);
}
