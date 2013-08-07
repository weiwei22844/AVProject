// SampleGrabberCB.cpp: implementation of the CSampleGrabberCB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "simpleplayer.h"
#include "SampleGrabberCB.h"
#include "TList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Global data
BOOL g_bOneShot=FALSE;
DWORD g_dwGraphRegister=0;  // For running object table
HWND g_hwnd=0;
CTList m_list;
DWORD dwNodeNum = 0;
CALLBACKINFO cb={0};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSampleGrabberCB::CSampleGrabberCB()
{
    pOwner = NULL;
    ZeroMemory(m_szCapDir, sizeof(m_szCapDir));
    ZeroMemory(m_szSnappedName, sizeof(m_szSnappedName));
    bFileWritten = FALSE;
}

CSampleGrabberCB::~CSampleGrabberCB()
{
    
}

STDMETHODIMP CSampleGrabberCB::BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize )
{
    // this flag will get set to true in order to take a picture
    //
    if( !g_bOneShot )
        return 0;
    
    if (!pBuffer)
        return E_POINTER;

    if(dwNodeNum >= 100)
    {
        return S_OK;
    }
    pNode pN = NULL;
    
    pN = new LNode;
    if(pN == NULL)
    {
        MessageBox(NULL, _T("new LNode failed"), NULL, MB_OK);
        return S_FALSE;
    }
    
    memset(pN, 0, sizeof(LNode));
    pN->pbBuffer = new BYTE[lBufferSize];
    if(pN->pbBuffer == NULL)
    {
        MessageBox(NULL, _T("alloc memory for new node failed"), NULL, MB_OK);
        delete pN;
        return S_FALSE;
    }
    
    memcpy(pN->pbBuffer, pBuffer, lBufferSize);
    pN->lSize = lBufferSize;
    pN->dwTimeStamp = dblSampleTime;
    pN->next = NULL;
    
    m_list.Insert(pN);
    dwNodeNum++;
    
    // Post a message to our application, telling it to come back
    // and write the saved data to a bitmap file on the user's disk.
    PostMessage(g_hwnd, WM_CAPTURE_BITMAP, 0, 0L);
    return 0;
}

BOOL CSampleGrabberCB::DisplayCapturedBits(BYTE *pBuffer, BITMAPINFOHEADER *pbih)
{
    // If we haven't yet snapped a still, return
    if (!bFileWritten || !pOwner || !pBuffer)
        return FALSE;
    
    // put bits into the preview window with StretchDIBits
    //
    HWND hwndStill = NULL;
    /*pOwner->GetDlgItem(IDC_STILL, &hwndStill );
    
      RECT rc;
      ::GetWindowRect( hwndStill, &rc );
      long lStillWidth = rc.right - rc.left;
      long lStillHeight = rc.bottom - rc.top;
      
        HDC hdcStill = GetDC( hwndStill );
        PAINTSTRUCT ps;
        BeginPaint(hwndStill, &ps);
        
          SetStretchBltMode(hdcStill, COLORONCOLOR);
          StretchDIBits( 
          hdcStill, 0, 0, 
          lStillWidth, lStillHeight, 
          0, 0, lWidth, lHeight, 
          pBuffer, 
          (BITMAPINFO*) pbih, 
          DIB_RGB_COLORS, 
          SRCCOPY );
          
            EndPaint(hwndStill, &ps);
    ReleaseDC( hwndStill, hdcStill );*/
    
    return TRUE;
}

// This is the implementation function that writes the captured video
// data onto a bitmap on the user's disk.
//
BOOL CSampleGrabberCB::CopyBitmap( double dblSampleTime, BYTE * pBuffer, long lBufferSize )
{
    if( !g_bOneShot || !pBuffer )
        return 0;
    
    // we only take one at a time
    //
    g_bOneShot = TRUE;                 // make change there by zww
    
    // figure out where to capture to
    //
    TCHAR m_ShortName[MAX_PATH];
    
    wsprintf( m_szSnappedName, TEXT("%sStillCap%4.4ld.bmp\0"), 
        m_szCapDir, pOwner->m_nCapTimes );
    
    wsprintf( m_ShortName, TEXT("StillCap%4.4ld.bmp\0"), 
        pOwner->m_nCapTimes );
    
    pOwner->m_nCapTimes++;
    if(pOwner->m_nCapTimes > 20)
    {
        g_bOneShot = FALSE;
    }
    
    // write out a BMP file
    //
    HANDLE hf = CreateFile(
        m_szSnappedName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
        CREATE_ALWAYS, NULL, NULL );
    
    if( hf == INVALID_HANDLE_VALUE )
        return 0;
    
    // write out the file header
    //
    BITMAPFILEHEADER bfh;
    memset( &bfh, 0, sizeof( bfh ) );
    bfh.bfType = 'MB';
    bfh.bfSize = sizeof( bfh ) + lBufferSize + sizeof( BITMAPINFOHEADER );
    bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );
    
    DWORD dwWritten = 0;
    WriteFile( hf, &bfh, sizeof( bfh ), &dwWritten, NULL );
    
    // and the bitmap format
    //
    BITMAPINFOHEADER bih;
    memset( &bih, 0, sizeof( bih ) );
    bih.biSize = sizeof( bih );
    bih.biWidth = lWidth;
    bih.biHeight = lHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    
    dwWritten = 0;
    WriteFile( hf, &bih, sizeof( bih ), &dwWritten, NULL );
    
    // and the bits themselves
    //
    dwWritten = 0;
    WriteFile( hf, pBuffer, lBufferSize, &dwWritten, NULL );
    CloseHandle( hf );
    bFileWritten = TRUE;
    
    // Display the bitmap bits on the dialog's preview window
    DisplayCapturedBits(pBuffer, &bih);
    
    // Save bitmap header for later use when repainting the window
    memcpy(&(cb.bih), &bih, sizeof(bih));        
    
    // show where it captured
    //
    //pOwner->SetDlgItemText( IDC_SNAPNAME, m_ShortName );
    
    // Enable the 'View Still' button
    /*HWND hwndButton = NULL;
    pOwner->GetDlgItem( IDC_BUTTON_VIEWSTILL, &hwndButton );
    ::EnableWindow(hwndButton, TRUE);
    
    // play a snap sound
    if (pOwner->IsDlgButtonChecked(IDC_PLAYSOUND))
    {
        PlaySnapSound();
    }*/
    
    return 0;
}

void CSampleGrabberCB::PlaySnapSound(void)
{
    TCHAR szSound[MAX_PATH];
    const TCHAR szFileToPlay[] = { TEXT("\\media\\click.wav\0") };
    int nSpaceAllowed = MAX_PATH - NUMELMS(szFileToPlay);
    int nSpaceUsed=0;
    
    nSpaceUsed = GetWindowsDirectory(szSound, nSpaceAllowed);
    if (nSpaceUsed && nSpaceUsed <= nSpaceAllowed)
    {
        _tcscat( szSound, szFileToPlay );
        sndPlaySound(szSound, SND_ASYNC);
    }           
}