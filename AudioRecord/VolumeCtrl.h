// VolumeCtrl.h: interface for the CVolumeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VOLUMECTRL_H__B6896D28_B52A_4E82_B1AE_099B8E552094__INCLUDED_)
#define AFX_VOLUMECTRL_H__B6896D28_B52A_4E82_B1AE_099B8E552094__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma once
#include "IVolume.h"

typedef BOOL (CALLBACK *PINPUTLINEPROC)	(UINT uLineIndex, MIXERLINE* pLineInfo, DWORD dwUserValue);

class CVolumeCtrl: public IVolume  
{
public:
	BOOL	GetMicrophoneSourceLineIndex(UINT* puLineIndex);
	static	BOOL	EnumerateInputLines( DWORD dwDeviceID, PINPUTLINEPROC, DWORD dwUserValue);

public:
	virtual BOOL	IsAvailable();
	virtual void	Enable();
	virtual void	Disable();
	virtual DWORD	GetVolumeMetric();
	virtual DWORD	GetMinimalVolume();
	virtual DWORD	GetMaximalVolume();
	virtual DWORD	GetCurrentVolume();
	virtual void	SetCurrentVolume(DWORD dwValue);

public:
	CVolumeCtrl(UINT uLineIndex, DWORD dwDeviceID=WAVE_MAPPER);
	virtual ~CVolumeCtrl();

private:
	DWORD m_dwDeviceID;
	// Status Info
	BOOL	m_bOK;
	BOOL	m_bInitialized;
	BOOL	m_bAvailable;

	// Mixer Info
	UINT	m_uMixerID;
	DWORD	m_dwMixerHandle;

	DWORD	m_dwLineID;
	DWORD	m_dwVolumeControlID;
	int		m_nChannelCount;
	UINT	m_uSourceLineIndex;

	UINT	m_uMicrophoneSourceLineIndex;
	
	HWND	m_hWnd;

	DWORD	m_dwMinimalVolume;
	DWORD	m_dwMaximalVolume;
	DWORD	m_dwVolumeStep;

private:
	BOOL	Init();
	void    Clear();

    BOOL	Initialize(UINT uLineIndex);

	static LRESULT CALLBACK MixerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef CVolumeCtrl* PCVolumeCtrl;

#endif // !defined(AFX_VOLUMECTRL_H__B6896D28_B52A_4E82_B1AE_099B8E552094__INCLUDED_)
