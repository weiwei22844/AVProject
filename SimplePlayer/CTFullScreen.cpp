// CTFullScreen.cpp : implementation file
//

#include "stdafx.h"
#include "CTFullScreen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTFullScreen

CTFullScreen::CTFullScreen()
{
	m_hParentWnd = NULL;
	m_hMsgDrainWnd = NULL;
}

CTFullScreen::~CTFullScreen()
{
}


BEGIN_MESSAGE_MAP(CTFullScreen, CWnd)
	//{{AFX_MSG_MAP(CTFullScreen)
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTFullScreen message handlers


BOOL CTFullScreen::Create(HWND hParentWnd,HWND hMsgDrainWnd)
{
	CRect rect(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));

	m_hParentWnd = hParentWnd;
	m_hMsgDrainWnd = hMsgDrainWnd;
	return CreateEx(NULL, 
				AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
							NULL, 
							(HBRUSH)GetStockObject(BLACK_BRUSH), 
							NULL), 
				NULL, 
				WS_POPUP|WS_VISIBLE, 
				rect.left,
				rect.top,
				rect.Width(),
				rect.Height(),
				hParentWnd, 
				NULL, 
				NULL);	
}

void CTFullScreen::OnDestroy() 
{
	CWnd::OnDestroy();
	// TODO: Add your message handler code here	
}


LRESULT CTFullScreen::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_SYSCOMMAND) 
	{
		if (wParam == SC_CLOSE)
		{
			if (m_hMsgDrainWnd)
			{
				//::PostMessage(m_hMsgDrainWnd,WM_USER_SYSTEM, wParam, lParam);
			}

			return 0;
		}			
	}
		
	return CWnd::WindowProc(message, wParam, lParam);
}



BOOL CTFullScreen::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
//	if (m_hMsgDrainWnd)
//		::PostMessage(m_hMsgDrainWnd,WM_PLAYENG_MOUSEWHEEL, MAKELONG(nFlags, zDelta), MAKELONG(pt.x, pt.y));

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
