// VolumeCtrl.cpp: implementation of the CVolumeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RecordAudio.h"
#include "VolumeCtrl.h"

#pragma comment ( lib, "winmm.lib" )

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define	BAD_DWORD		(DWORD)-1

#define WND_CLASS_NAME  "Volume Class Name"
#define WND_NAME        "Volume Name"

#define AUDFREQ			22050	// Frequency
#define AUDCHANNELS		1		// Number of channels
#define AUDBITSMPL		16		// Number of bits per sample

PCVolumeCtrl G_pThis = NULL;

inline void SetDeviceType(WAVEFORMATEX* pwfe)
{
	memset( pwfe, 0, sizeof(WAVEFORMATEX) );
	WORD  nBlockAlign = (AUDCHANNELS*AUDBITSMPL)/8;
	DWORD nSamplesPerSec = AUDFREQ;
	pwfe->wFormatTag = WAVE_FORMAT_PCM;
	pwfe->nChannels = AUDCHANNELS;
	pwfe->nBlockAlign = nBlockAlign;
	pwfe->nSamplesPerSec = nSamplesPerSec;
	pwfe->wBitsPerSample = AUDBITSMPL;
	pwfe->nAvgBytesPerSec = nSamplesPerSec*nBlockAlign;
}

CVolumeCtrl::CVolumeCtrl(UINT uLineIndex, DWORD dwDeviceID)
						:m_dwDeviceID(dwDeviceID),
						 m_bOK(FALSE),
						 m_bInitialized(FALSE), 
						 m_bAvailable(false),
						 m_uMixerID(0L),
						 m_dwMixerHandle(0L),
						 m_dwLineID(0L),
						 m_dwVolumeControlID(0L),
						 m_nChannelCount(0L),
						 m_uSourceLineIndex(0L),
						 m_uMicrophoneSourceLineIndex(BAD_DWORD), 
						 m_hWnd(NULL),
						 m_dwMinimalVolume(BAD_DWORD),
						 m_dwMaximalVolume(BAD_DWORD),
						 m_dwVolumeStep(0L)
{
	if (m_bOK = Init())
	{
		G_pThis = this;
		if (!Initialize(uLineIndex))
		{
			Clear();
			G_pThis = NULL;
		}		
	}
}

CVolumeCtrl::~CVolumeCtrl()
{
	if (m_bOK)
	{
		Clear();
	}
	G_pThis = NULL;
}

BOOL CVolumeCtrl::IsAvailable()
{
	return m_bAvailable;
}

void CVolumeCtrl::Enable()
{
	if ( !m_bInitialized || !IsAvailable() )
		return;
	BOOL bAnyEnabled = FALSE;

	MMRESULT mmResult;

	MIXERLINE lineDestination;
	memset( &lineDestination, 0, sizeof(MIXERLINE) );
	lineDestination.cbStruct = sizeof(MIXERLINE);
	lineDestination.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	mmResult = mixerGetLineInfo( (HMIXEROBJ)m_dwMixerHandle, &lineDestination, MIXER_GETLINEINFOF_COMPONENTTYPE );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		TRACE(".InputXxxVolume: FAILURE: Could not get the Destination Line while enabling. mmResult=%d\n", mmResult );
		return;
	}

	// Getting all line's controls
	int nControlCount = lineDestination.cControls;
	int nChannelCount = lineDestination.cChannels;
	MIXERLINECONTROLS LineControls;
	memset( &LineControls, 0, sizeof(MIXERLINECONTROLS) );
	MIXERCONTROL* aControls = (MIXERCONTROL*)malloc( nControlCount * sizeof(MIXERCONTROL) );
	if ( !aControls )
	{
		TRACE(".InputXxxVolume: FAILURE: Out of memory while enabling the line.\n" );
		return;
	}
	memset( &aControls[0], 0, sizeof(nControlCount * sizeof(MIXERCONTROL)) );
	for ( int i = 0; i < nControlCount; i++ )
	{
		aControls[i].cbStruct = sizeof(MIXERCONTROL);
	}
	LineControls.cbStruct = sizeof(MIXERLINECONTROLS);
	LineControls.dwLineID = lineDestination.dwLineID;
	LineControls.cControls = nControlCount;
	LineControls.cbmxctrl = sizeof(MIXERCONTROL);
	LineControls.pamxctrl = &aControls[0];
	           
	mmResult = mixerGetLineControls((HMIXEROBJ)m_dwMixerHandle, &LineControls, MIXER_GETLINECONTROLSF_ALL);
	if (mmResult == MMSYSERR_NOERROR)
	{
		for ( i = 0; i < nControlCount; i++ )
		{
			if ( aControls[i].dwControlType & MIXERCONTROL_CT_UNITS_BOOLEAN )
			{
				MIXERCONTROLDETAILS_BOOLEAN* aDetails = NULL;
				int nMultipleItems = aControls[i].cMultipleItems;
				int nChannels = nChannelCount;
				// MIXERCONTROLDETAILS
				MIXERCONTROLDETAILS ControlDetails;
				memset( &ControlDetails, 0, sizeof(MIXERCONTROLDETAILS) );
				ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
				ControlDetails.dwControlID = aControls[i].dwControlID;
				if ( aControls[i].fdwControl & MIXERCONTROL_CONTROLF_UNIFORM )
				{
					nChannels = 1;
				}
				if ( aControls[i].fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE )
				{
					nMultipleItems = aControls[i].cMultipleItems;
					aDetails = (MIXERCONTROLDETAILS_BOOLEAN*)malloc(nMultipleItems*nChannels*sizeof(MIXERCONTROLDETAILS_BOOLEAN));
					if ( !aDetails )
					{
						TRACE(".InputXxxVolume: FAILURE: Out of memory while enabling the line.\n" );
						continue;
					}
					for ( int nItem = 0; nItem < nMultipleItems; nItem++ )
					{
						LONG lValue = FALSE;
						if ( nItem == (int)(nMultipleItems-m_uSourceLineIndex))
						{
							lValue = TRUE;
						}
						for ( int nChannel = 0; nChannel < nChannels; nChannel++ )
						{
							aDetails[nItem+nChannel].fValue = lValue;
						}
					}
				}

				ControlDetails.cChannels = nChannels;
				ControlDetails.cMultipleItems = nMultipleItems;
				ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
				ControlDetails.paDetails = &aDetails[0];

				mmResult = mixerSetControlDetails( (HMIXEROBJ)m_dwMixerHandle, &ControlDetails, 0L );
				if ( mmResult == MMSYSERR_NOERROR )
				{
					TRACE(".InputXxxVolume: Enabling Line: Line control \"%s\" has been enabled.\n", aControls[i].szShortName );
					bAnyEnabled = TRUE;
				}
				free( aDetails );
			}
		}
	} 
	else 
	{
		TRACE(".InputXxxVolume: FAILURE: Could not get the line's controls while enabling. mmResult=%d\n", mmResult );
	}
	free( aControls );
	if ( !bAnyEnabled )
	{
		TRACE(".InputXxxVolume: WARNING: No controls were found for enabling the line.\n" );
	}
}

void CVolumeCtrl::Disable()
{

}

DWORD CVolumeCtrl::GetVolumeMetric()
{
	if (!m_bAvailable)
	{
		return BAD_DWORD;
	}
	return m_dwVolumeStep;
}

DWORD CVolumeCtrl::GetMinimalVolume()
{
	if (!m_bAvailable)
	{
		return BAD_DWORD;
	}
	return m_dwMinimalVolume;
}

DWORD CVolumeCtrl::GetMaximalVolume()
{
	if (!m_bAvailable)
	{
		return BAD_DWORD;
	}
	return m_dwMaximalVolume;
}

DWORD CVolumeCtrl::GetCurrentVolume()
{
	if (!m_bAvailable)
	{
		return BAD_DWORD;
	}

	MIXERCONTROLDETAILS_UNSIGNED* aDetails = (MIXERCONTROLDETAILS_UNSIGNED*)malloc(m_nChannelCount*sizeof(MIXERCONTROLDETAILS_UNSIGNED));
	if (!aDetails)
	{
		return BAD_DWORD;
	}

	MIXERCONTROLDETAILS ControlDetails;
	memset( &ControlDetails, 0, sizeof(MIXERCONTROLDETAILS) );
	ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	ControlDetails.dwControlID = m_dwVolumeControlID;
	ControlDetails.cChannels = m_nChannelCount;
	ControlDetails.cMultipleItems = 0;
	ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	ControlDetails.paDetails = &aDetails[0];

	MMRESULT mmResult = mixerGetControlDetails( (HMIXEROBJ)m_dwMixerHandle, &ControlDetails, MIXER_GETCONTROLDETAILSF_VALUE );
	DWORD dw = aDetails[0].dwValue;
	free(aDetails);
	if ( mmResult != MMSYSERR_NOERROR )
	{
		return BAD_DWORD;
	}

	return dw;
}

void CVolumeCtrl::SetCurrentVolume(DWORD dwValue)
{
	if (!m_bAvailable || (dwValue<m_dwMinimalVolume) || (dwValue>m_dwMaximalVolume))
	{
		return;
	}

	MIXERCONTROLDETAILS_UNSIGNED* aDetails = (MIXERCONTROLDETAILS_UNSIGNED*)malloc(m_nChannelCount*sizeof(MIXERCONTROLDETAILS_UNSIGNED));
	if (!aDetails)
	{
		return;
	}

	for (int i = 0; i < m_nChannelCount; i++)
	{
		aDetails[i].dwValue = dwValue;
	}

	MIXERCONTROLDETAILS ControlDetails;
	memset( &ControlDetails, 0, sizeof(MIXERCONTROLDETAILS) );
	ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	ControlDetails.dwControlID = m_dwVolumeControlID;
	ControlDetails.cChannels = m_nChannelCount;
	ControlDetails.cMultipleItems = 0;
	ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	ControlDetails.paDetails = &aDetails[0];

	MMRESULT mmResult = mixerSetControlDetails( (HMIXEROBJ)m_dwMixerHandle, &ControlDetails, MIXER_SETCONTROLDETAILSF_VALUE );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		TRACE(".InputXxxVolume: FAILURE: Could not set volume(%d) mmResult=%d\n", dwValue, mmResult );
	}

	free(aDetails);
}

BOOL CVolumeCtrl::Init()
{
	if (!mixerGetNumDevs())
	{
		return FALSE;
	}

	HWAVEIN hwaveIn;
	MMRESULT mmResult;
	WAVEFORMATEX WaveFmt;
	SetDeviceType( &WaveFmt );
	mmResult = waveInOpen(&hwaveIn, m_dwDeviceID, &WaveFmt, 0L, 0L, CALLBACK_NULL);
	if (mmResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	else
	{
		// 得到混响器的标识号
		mmResult = mixerGetID((HMIXEROBJ)hwaveIn, &m_uMixerID, MIXER_OBJECTF_HWAVEIN);
		waveInClose(hwaveIn);
		if (mmResult != MMSYSERR_NOERROR )
		{
			return FALSE;
		}
	}

	// Exposing Window to Mixer
	WNDCLASSEX wcx;
	memset( &wcx, 0, sizeof(WNDCLASSEX) );	
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.lpszClassName = WND_CLASS_NAME;
	wcx.lpfnWndProc = (WNDPROC)MixerWndProc;
	::RegisterClassEx(&wcx);
	m_hWnd = CreateWindow(WND_CLASS_NAME,
		                  WND_NAME, 
						  WS_POPUP | WS_DISABLED,
						  0, 0, 0, 0,
						  NULL, NULL, NULL, NULL );
	if (!m_hWnd)
	{
		return FALSE;
	}
	::ShowWindow(m_hWnd, SW_HIDE);
	::UpdateWindow(m_hWnd);

	//
	// 打开混响器设备
	// 参数:
	//      m_dwMixerHandle--混响器设备的句柄  
	//      m_uMixerID--为混响器的标识号,用于指定要打开的混响器设备
	//	
	mmResult = mixerOpen( (LPHMIXER)&m_dwMixerHandle, m_uMixerID, (DWORD)m_hWnd, 0L, CALLBACK_WINDOW );
	if (mmResult != MMSYSERR_NOERROR || m_uMixerID == -1)
	{
		::DestroyWindow( m_hWnd );
		return FALSE;
	}

	return TRUE;
}

BOOL CVolumeCtrl::Initialize(UINT uLineIndex)
{
	if (!m_bOK)
	{
		return FALSE;
	}

	MIXERLINE MixerLine;
	memset( &MixerLine, 0, sizeof(MIXERLINE) );
	MixerLine.cbStruct = sizeof(MIXERLINE);
	MixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
						
	MMRESULT mmResult = mixerGetLineInfo((HMIXEROBJ)m_dwMixerHandle, 
		                                 &MixerLine, 
										 MIXER_GETLINEINFOF_COMPONENTTYPE);

	if (mmResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}


	MIXERCONTROL Control;
	memset( &Control, 0, sizeof(MIXERCONTROL) );
	Control.cbStruct = sizeof(MIXERCONTROL);

	MIXERLINECONTROLS LineControls;
	memset( &LineControls, 0, sizeof(MIXERLINECONTROLS) );
	LineControls.cbStruct = sizeof(MIXERLINECONTROLS);

	MIXERLINE Line;
	memset( &Line, 0, sizeof(MIXERLINE) );
	Line.cbStruct = sizeof(MIXERLINE);

	if (uLineIndex < MixerLine.cConnections)
	{
		Line.dwDestination = MixerLine.dwDestination;
		Line.dwSource = uLineIndex;
		mmResult = mixerGetLineInfo( (HMIXEROBJ)m_dwMixerHandle, &Line, MIXER_GETLINEINFOF_SOURCE );
		if ( mmResult != MMSYSERR_NOERROR )
		{
			return FALSE;
		}

		LineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		LineControls.dwLineID = Line.dwLineID;
		LineControls.cControls = 1;
		LineControls.cbmxctrl = sizeof(MIXERCONTROL);
		LineControls.pamxctrl = &Control;
		mmResult = mixerGetLineControls((HMIXEROBJ)m_dwMixerHandle, &LineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
		if (mmResult == MMSYSERR_NOERROR)
		{
			if (!(Control.fdwControl & MIXERCONTROL_CONTROLF_DISABLED))
			{
				m_bAvailable = TRUE;
			} 
		} 
	} 
	else 
	{
		return FALSE;
	}

	/*
	// Retrieving Microphone Source Line
	for ( UINT uLine = 0; uLine < MixerLine.cConnections; uLine++ )
	{
		MIXERLINE MicrophoneLine;
		memset( &MicrophoneLine, 0, sizeof(MIXERLINE) );
		MicrophoneLine.cbStruct = sizeof(MIXERLINE);
		MicrophoneLine.dwDestination = MixerLine.dwDestination;
		MicrophoneLine.dwSource = uLine;
		mmResult = mixerGetLineInfo( (HMIXEROBJ)m_dwMixerHandle, &MicrophoneLine, MIXER_GETLINEINFOF_SOURCE );
		if ( mmResult == MMSYSERR_NOERROR )
		{
			if ( MicrophoneLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE )
			{
				m_uMicrophoneSourceLineIndex = MicrophoneLine.dwSource;
				break;
			}
		}
	}
	if ( m_uMicrophoneSourceLineIndex == BAD_DWORD )
	{
		TRACE(".InputXxxVolume: WARNING: Could not retrieve Microphone Source Line.\n" );
	}
	*/

	m_uSourceLineIndex = uLineIndex+1;
	m_nChannelCount = Line.cChannels;
	m_dwLineID = LineControls.dwLineID;
	m_dwVolumeControlID = Control.dwControlID;
	m_dwMinimalVolume = Control.Bounds.dwMinimum;
	m_dwMaximalVolume = Control.Bounds.dwMaximum;
	m_dwVolumeStep = Control.Metrics.cSteps;

	m_bInitialized = TRUE;
	return TRUE;
}

BOOL CVolumeCtrl::GetMicrophoneSourceLineIndex(UINT* puLineIndex)
{
	if (!puLineIndex || !m_bInitialized || (m_uMicrophoneSourceLineIndex==BAD_DWORD))
	{
		return FALSE;
	}

	*puLineIndex = m_uMicrophoneSourceLineIndex;
	return TRUE;
}

void CVolumeCtrl::Clear()
{
	if (mixerClose((HMIXER)m_dwMixerHandle ) != MMSYSERR_NOERROR)
	{
		TRACE("CVolumeCtrl WARNING: Could not close Mixer.\r\n");
	}
	::DestroyWindow( m_hWnd );
	m_bInitialized = FALSE;
	m_bOK = FALSE;	
}

BOOL CVolumeCtrl::EnumerateInputLines( DWORD dwDeviceID, PINPUTLINEPROC pUserCallback, DWORD dwUserValue )
{
	if (!pUserCallback)
	{
		return FALSE;
	}

	MMRESULT mmResult;
	HWAVEIN hwaveIn;
	WAVEFORMATEX WaveFmt;
	SetDeviceType( &WaveFmt );
	mmResult = waveInOpen( &hwaveIn, dwDeviceID, &WaveFmt, 0L, 0L, CALLBACK_NULL );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		return FALSE;
	}

	UINT uMixerID;
	DWORD dwMixerHandle;
	mmResult = mixerGetID( (HMIXEROBJ)hwaveIn, &uMixerID, MIXER_OBJECTF_HWAVEIN );
	waveInClose( hwaveIn );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		return FALSE;
	}
	mmResult = mixerOpen( (LPHMIXER)&dwMixerHandle, uMixerID, 0L, 0L, 0L );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		mixerClose( (HMIXER)dwMixerHandle );
		return FALSE;
	}
	MIXERLINE MixerLine;
	memset( &MixerLine, 0, sizeof(MIXERLINE) );
	MixerLine.cbStruct = sizeof(MIXERLINE);
	MixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	mmResult = mixerGetLineInfo( (HMIXEROBJ)dwMixerHandle, &MixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		mixerClose( (HMIXER)dwMixerHandle );
		return FALSE;
	}
	MIXERLINE Line;
	for ( UINT uLineIndex = 0; uLineIndex < MixerLine.cConnections; uLineIndex++ )
	{
		memset( &Line, 0, sizeof(MIXERLINE) );
		Line.cbStruct = sizeof(MIXERLINE);
		Line.dwDestination = MixerLine.dwDestination;
		Line.dwSource = uLineIndex;
		mmResult = mixerGetLineInfo( (HMIXEROBJ)dwMixerHandle, &Line, MIXER_GETLINEINFOF_SOURCE );
		if ( mmResult != MMSYSERR_NOERROR )
		{
			mixerClose( (HMIXER)dwMixerHandle );
			return FALSE;
		}
		if ( !((*pUserCallback)( uLineIndex, &Line, dwUserValue )) )
		{
			break;
		}
	}
	mixerClose( (HMIXER)dwMixerHandle );

	return TRUE;
}

LRESULT CALLBACK CVolumeCtrl::MixerWndProc(HWND hwnd, UINT uMsg, 
										   WPARAM wParam, LPARAM lParam)
{

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}