// AudioCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "RecordAudio.h"
#include "AudioCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioCtrl

CAudioCtrl::CAudioCtrl()
: m_uiDeviceID(0)
, m_eStatus(ENUM_STATUS_INVALID)
, m_hRecord(NULL)
, m_hPlay(NULL)
, m_dwQueuBufferSize(1024)
, m_nDataQueueNum(0)
, m_bRecording(FALSE)
, m_eRecChannel(ENUM_REC_CHANNEL_MONO)
, m_szAryInData(NULL)
, m_szLeftInData(NULL)
, m_szRightInData(NULL)
, m_pAryHdr (NULL)
, m_uiMixerID(0)
{
	memset(&m_Format, 0, sizeof(WAVEFORMATEX));
	m_nDataQueueNum = 0;

	ZeroMemory(&m_MMCKInfoParent,sizeof(m_MMCKInfoParent));
	ZeroMemory(&m_MMCKInfoChild,sizeof(m_MMCKInfoChild));
	memset ( m_hWaveFile, 0, sizeof(m_hWaveFile) );

	SetBkColor(RGB(0, 0, 0));

	m_bAlwaysDrawTowChannel = false;
}

CAudioCtrl::~CAudioCtrl()
{
	StopAndFreeAll();
}


BEGIN_MESSAGE_MAP(CAudioCtrl, CWnd)
	//{{AFX_MSG_MAP(CAudioCtrl)
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAudioCtrl message handlers

//
// 获得声卡的数量
//
UINT CAudioCtrl::GetWaveInDevs()
{
	return waveInGetNumDevs();
}

char* CAudioCtrl::GetWaveInName(UINT uiDevID)
{
	WAVEINCAPS tagCaps;
	if (waveInGetDevCaps(uiDevID, &tagCaps, sizeof(tagCaps)) == MMSYSERR_NOERROR)
	{
		return tagCaps.szPname;
	}
	return NULL;
}

UINT CAudioCtrl::GetWaveInAvailableDev()
{
	for ( DWORD i=0; i<GetWaveInDevs(); i++ )
	{
		CVolumeCtrl VolumeIn( 0, i );
		if (VolumeIn.IsAvailable())
		{
			return i;
		}
	}

	return -1;
}

void CAudioCtrl::SetDeviceID(UINT uiIndex)
{
	m_uiDeviceID = uiIndex;
}

UINT CAudioCtrl::InitCtrl(UINT uiDevIndex,
						  DWORD dwBufSize, 
						  HWND hwndParent, 
						  LPRECT lpRect, 
						  MsgNotifyProc msgNodityProc, 
						  WPARAM wParam)
{
	StopAndFreeAll();
	SetDeviceID(uiDevIndex);
	m_dwQueuBufferSize	= dwBufSize;
	m_pMsgNotifyProc	= msgNodityProc;
	m_wParam = wParam;

	Create(hwndParent, lpRect);

	/********** 绘图**********/
	if ( hwndParent && lpRect )
	{
		m_PenG.CreatePen ( PS_SOLID, 0, RGB(0, 255, 0) );
		m_PenPartLine.CreatePen ( PS_SOLID, PARTLINE_HEIGHT, COLOR_FRAME );
	}
	/********** 绘图**********/

	m_eStatus = ENUM_STATUS_READY;

	return TRUE;
}

//
// 获得播放声音的声卡数量
//
UINT CAudioCtrl::GetWaveOutCount()
{
	return waveOutGetNumDevs();
}

BOOL CAudioCtrl::Create(HWND hwndParent, LPRECT lpRect)
{
	LPCTSTR lpszClassName = AfxRegisterWndClass(
		0, LoadCursor(AfxGetInstanceHandle(), IDC_ARROW),
		NULL, NULL );
	
	CRect rc ( 0,0,0,0 );
	if ( lpRect ) rc = *lpRect;
	
	if ( !CreateEx ( 0, lpszClassName, "",
		WS_CHILD | WS_TABSTOP,
		rc.left, rc.top, rc.Width(), rc.Height(),
		hwndParent, NULL, NULL) )
	{
		AfxMessageBox ( "Create window failed" );
		return FALSE;
	}
	
	if ( lpRect )
		ShowWindow ( SW_SHOW );
	else
		ShowWindow ( SW_HIDE );

	GetClientRect (&m_rcClient );
	

	/********** 绘图**********/
	CClientDC dc(this);
	m_fntChannelText.CreatePointFont ( 100, "Impact", &dc );
//	m_fntDeviceNameText.CreateFont ( 14, 0, 0, 0, 0, TRUE, TRUE, FALSE, 0, 0,
//		0, 0, 0, "MS Sans Serif" );
	/********** 绘图**********/

	return TRUE;
}

BOOL CAudioCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message >= MM_WOM_OPEN) && (pMsg->message <= MM_MOM_DONE))
	{
		switch ( pMsg->message )
		{
		case MM_WIM_DATA:
			OnMM_WIM_DATA ( pMsg->wParam, pMsg->lParam );
			break;
		case MM_WIM_CLOSE:
			m_eStatus = ENUM_STATUS_READY;
			m_nDataQueueNum = 0;
			m_bRecording = FALSE;
			break;
		case MM_WOM_DONE:
			OnMM_WOM_DONE ( pMsg->wParam, pMsg->lParam );
			break;
		case MM_WOM_CLOSE:
			m_eStatus = ENUM_STATUS_READY;
			m_bRecording = FALSE;
			break;
		case MM_WIM_OPEN:
			break;
		case MM_WOM_OPEN:
			break;
		}
		
		if (m_pMsgNotifyProc)
		{
			m_pMsgNotifyProc(pMsg->message, m_wParam);
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

//
// 录音对应当数据缓冲写满后触发的消息
//
LRESULT CAudioCtrl::OnMM_WIM_DATA (WPARAM wParam, LPARAM lParam)
{
	MMRESULT mmReturn = 0;
	
	LPWAVEHDR pHdr = (LPWAVEHDR) lParam;
	ASSERT ( pHdr );

	// 清除波形输入缓冲区
	mmReturn = ::waveInUnprepareHeader (m_hRecord, pHdr, sizeof(WAVEHDR));
	if (mmReturn)
	{
		return -1L;
	}

	if(m_eStatus == ENUM_STATUS_RECORDING)
	{
		// 提取单声道PCM数据
		int nBytesPickup = PickupMonoData(m_Format.wBitsPerSample, pHdr->lpData, pHdr->dwBytesRecorded);

		// 根据需要保存的通道文件类型选择PCM数据和数据长度
		char *pRecData[ENUM_FILE_CHANNEL_NUM] = { pHdr->lpData, pHdr->lpData };
		int nRecBytes[ENUM_FILE_CHANNEL_NUM] = { pHdr->dwBytesRecorded, pHdr->dwBytesRecorded };

		if ( m_eRecChannel == ENUM_REC_CHANNEL_ALONE )
		{
			pRecData[ENUM_FILE_CHANNEL_LEFT] = m_szLeftInData;
			nRecBytes[ENUM_FILE_CHANNEL_LEFT] = nBytesPickup;
			pRecData[ENUM_FILE_CHANNEL_RIGHT] = m_szRightInData;
			nRecBytes[ENUM_FILE_CHANNEL_RIGHT] = nBytesPickup;
		}

		// 保存到wave文件中
		for ( int eFileChannel=ENUM_FILE_CHANNEL_COMMON; eFileChannel<ENUM_FILE_CHANNEL_NUM; eFileChannel++ )
		{
			if ( m_hWaveFile[eFileChannel] )
			{
				int length = ::mmioWrite(m_hWaveFile[eFileChannel], pRecData[eFileChannel], nRecBytes[eFileChannel] );
				if ( length != nRecBytes[eFileChannel] )
				{
					Stop ();
					m_nDataQueueNum --;
					TRACE("write file failed\r\n");
					return -1L;
				}
			}
		}

		// 为波形输入准备一个输入缓冲区
		mmReturn = ::waveInPrepareHeader (m_hRecord, pHdr, sizeof(WAVEHDR));
		if (mmReturn)
		{
			TRACE("waveInPrepareHeader failed\r\n");
		}
		else
		{
			// 给输入设备增加一个缓存
			mmReturn = ::waveInAddBuffer(m_hRecord, pHdr, sizeof(WAVEHDR));
			if (mmReturn)
			{
				TRACE("waveInAddBuffer() failed\r\n");
			}
			else
			{
				DrawWave( (DWORD)nBytesPickup );
				return S_OK;
			}
		}
	}
	else
	{
		if (m_nDataQueueNum == 1)
		{
			StopRec();
		}
		else
		{
			m_nDataQueueNum --;
		}
	}

	return S_OK;
}

//
// 播放对应当数据缓冲读完后触发的消息
//
LRESULT CAudioCtrl::OnMM_WOM_DONE (WPARAM wParam, LPARAM lParam)
{
	//TRACE("OnMM_WOM_DONE!\r\n");

	MMRESULT mmReturn = 0;
	
	LPWAVEHDR pHdr = (LPWAVEHDR)lParam;
	mmReturn = ::waveOutUnprepareHeader(m_hPlay, pHdr, sizeof(WAVEHDR));
	if ( mmReturn )
	{
		TRACE("waveOutUnprepareHeader() failed!\r\n" );
		return -1L;
	}
	
	m_nDataQueueNum--;

	if ( m_eStatus == ENUM_STATUS_PLAYING )
	{		
		
		int nSize = m_dwQueuBufferSize;
		if ( ReadSoundDataFromFile ( pHdr->lpData, nSize ) )
		{
			AddOutputBufferToQueue ( (int)pHdr->dwUser, nSize );			
			return 0L;
		}
		else
		{
			Stop();
		}
	}

	if ( m_nDataQueueNum == 0 && m_eStatus != ENUM_STATUS_PLAYING )
	{
		StopPlay ();
	}

	return S_OK;
}

//
// 从立体声PCM数据中提取单声道数据，结果保存到 m_szLeftInData 、 m_szRightInData 中
// return : ------------------------------------------------------------------------------
//		单声道数据长度（字节）
//
int CAudioCtrl::PickupMonoData(WORD wBitsPerSample, char *szOrgData, int nOrgSize)
{
	if ( m_Format.nChannels == 1 )
	{
		ASSERT ( m_dwQueuBufferSize >= (DWORD)nOrgSize );
		memcpy ( m_szLeftInData, szOrgData, nOrgSize );
		memcpy ( m_szRightInData, szOrgData, nOrgSize );
		return nOrgSize;
	}

	DWORD dwBytesPerSample = wBitsPerSample/8;
	int nDestBytes_Left = 0, nDestBytes_Right = 0;
	for ( int i=0; i<nOrgSize; i+=2*dwBytesPerSample )
	{
		memcpy ( m_szLeftInData+nDestBytes_Left, szOrgData+i, dwBytesPerSample );
		nDestBytes_Left += dwBytesPerSample;
		memcpy ( m_szRightInData+nDestBytes_Right, szOrgData+i+dwBytesPerSample, dwBytesPerSample );
		nDestBytes_Right += dwBytesPerSample;
	}
	ASSERT ( nDestBytes_Left == nDestBytes_Right );
	return nDestBytes_Left;
}

void CAudioCtrl::Stop()
{
	if ( m_eStatus != ENUM_STATUS_PLAYING && m_eStatus != ENUM_STATUS_RECORDING )
		return;

	MMRESULT mmReturn = 0;
	if ( m_eStatus == ENUM_STATUS_PLAYING )
	{
		if ( ::waveOutReset(m_hPlay) ) 
		{
			TRACE("waveOutReset failed\r\n");
		}
		SetTimer ( TIMER_EVENT_STOPPLAY, 1000, NULL );
	}
	else if ( m_eStatus == ENUM_STATUS_RECORDING )
	{
		SetTimer ( TIMER_EVENT_STOPREC, 1000, NULL );
	}
	
	Invalidate ( TRUE );
	m_eStatus = ENUM_STATUS_STOPING;
}

void CAudioCtrl::StopRec()
{
	if ( !m_hRecord ) return;
	if ( m_eStatus != ENUM_STATUS_RECORDING && m_eStatus != ENUM_STATUS_STOPING )
		return;

	MMRESULT mmReturn = 0;
	mmReturn = ::waveInReset ( m_hRecord );
	if ( mmReturn ) 
	{
		TRACE("waveInReset failed\r\n");	
	}

	::Sleep ( 10 );
	for ( int eFileChannel=ENUM_FILE_CHANNEL_COMMON; eFileChannel<ENUM_FILE_CHANNEL_NUM; eFileChannel++ )
	{
		//StopRecordAudioFile ( (ENUM_FILE_CHANNEL)eFileChannel, "mp3" );
		StopRecordAudioFile ( (ENUM_FILE_CHANNEL)eFileChannel, "wav" );
	}

	mmReturn = ::waveInClose ( m_hRecord );
	m_hRecord = NULL;
	if ( mmReturn ) 
	{
		TRACE("waveInClose failed\r\n" );	
	}
	//FreeMp3Encode ();
}

// 
// 录制
//
BOOL CAudioCtrl::Record(UINT uiRecChannel,   /*录制信道*/
						DWORD dwSamplingRate,/*采样率*/ 
						WORD  wSamplingBit  /*采样位*/
						) 
{
	// 录音设备正在录音
	if ( m_eStatus == ENUM_STATUS_RECORDING )
	{
		TRACE("recording...\r\n");
		return FALSE;
	}

	if (m_eStatus != ENUM_STATUS_READY)
	{
		TRACE("AudioCtrl state failed...\r\n");
		return FALSE;
	}

	m_bRecording = TRUE;
	MMRESULT mmReturn = 0;
	ASSERT((wSamplingBit%8) == 0);
	if (wSamplingBit > 16)
	{
		wSamplingBit = 16;
	}
	
	SetWaveFormat(uiRecChannel, dwSamplingRate, wSamplingBit);
	if (!SetRelateParaAfterGetWaveFormat())
	{
		return TRUE;
	}

	mmReturn = ::waveInOpen(&m_hRecord, m_uiDeviceID, &m_Format, (DWORD)GetSafeHwnd(), NULL, CALLBACK_WINDOW);
	if (mmReturn)
	{
		TRACE("waveInOpen is failed!\r\n");
		goto failed;
	}
	else
	{
		// make several input buffers and add them to the input queue
		for(int i=0; i<m_wInQueu; i++)
		{
			AddInputBufferToQueue ( i );
		}
		
		// start recording
		mmReturn = ::waveInStart ( m_hRecord );
		if ( mmReturn )
		{
			TRACE("waveInStart is failed!\r\n");
			goto failed;
		}
	}
	m_eStatus = ENUM_STATUS_RECORDING;

	return TRUE;
failed:
	FreeBuffer();
	return FALSE;
}

void CAudioCtrl::SetWaveFormat(UINT uiRecChannel, DWORD dwSamplingRate, WORD  wSamplingBit)
{
	m_eRecChannel = (ENUM_REC_CHANNEL)uiRecChannel;
	memset(&m_Format, 0, sizeof(WAVEFORMATEX));
	m_Format.cbSize				= 0;
	m_Format.wFormatTag			= WAVE_FORMAT_PCM;
	m_Format.wBitsPerSample		= wSamplingBit;
	m_Format.nChannels			= ((uiRecChannel==ENUM_REC_CHANNEL_MONO) ? 1 : 2);
	m_Format.nSamplesPerSec		= dwSamplingRate;
	m_Format.nAvgBytesPerSec	= m_Format.nSamplesPerSec * (m_Format.wBitsPerSample/8);
	m_Format.nBlockAlign		= m_Format.nChannels * (m_Format.wBitsPerSample/8);
}

void CAudioCtrl::FreeBuffer()
{
	// 录音或者播放尚未停止，不能释放内存
	for ( int eFileChannel=ENUM_FILE_CHANNEL_COMMON; eFileChannel<ENUM_FILE_CHANNEL_NUM; eFileChannel++ )
	{
		//if ( m_hWaveFile[eFileChannel] || m_pFileMp3[eFileChannel] )
		if (m_hWaveFile[eFileChannel])
			return;
	}

	if (m_szAryInData)
	{
		for ( int i=0; i<m_wInQueu; i++ )
		{
			if ( m_szAryInData[i] )
				delete[] m_szAryInData[i];
		}
		memset ( m_szAryInData, 0, sizeof(char*)*m_wInQueu );
		delete[] m_szAryInData;
		m_szAryInData = NULL;
	}

	if ( m_szLeftInData )
	{
		delete[] m_szLeftInData;
		m_szLeftInData = NULL;
	}

	if ( m_szRightInData )
	{
		delete[] m_szRightInData;
		m_szRightInData = NULL;
	}

	if (m_pAryHdr)
	{
		for (int i=0; i<m_wInQueu; i++)
		{
			if (m_pAryHdr[i])
			{
				delete[] m_pAryHdr[i];
			}
		}
		memset(m_pAryHdr, 0, sizeof(WAVEHDR*)*m_wInQueu);
		delete[] m_pAryHdr;
		m_pAryHdr = NULL;
	}
}

BOOL CAudioCtrl::SetRelateParaAfterGetWaveFormat()
{
	FreeBuffer ();
	m_wInQueu = (WORD) ( m_Format.nChannels + m_Format.wBitsPerSample/8 + m_Format.nSamplesPerSec/11025 );
	if ( !AllocateBuffer ( m_dwQueuBufferSize ) )
		return FALSE;
	return TRUE;
}

BOOL CAudioCtrl::AllocateBuffer(DWORD dwBufferSize)
{
	m_dwQueuBufferSize = dwBufferSize;

	ASSERT ( m_wInQueu > 0 );
	m_szAryInData = new char*[m_wInQueu];
	m_szLeftInData = new char[m_dwQueuBufferSize];
	m_szRightInData = new char[m_dwQueuBufferSize];
	m_pAryHdr = new WAVEHDR*[m_wInQueu];
	if ( !m_szAryInData || !m_szLeftInData || !m_szRightInData || !m_pAryHdr )
	{
		::AfxThrowMemoryException ();
		return FALSE;
	}
	memset ( m_szAryInData, 0, sizeof(char*)*m_wInQueu );
	memset ( m_szLeftInData, 0, sizeof(char)*m_dwQueuBufferSize );
	memset ( m_szRightInData, 0, sizeof(char)*m_dwQueuBufferSize );
	memset ( m_pAryHdr, 0, sizeof(WAVEHDR*)*m_wInQueu );

	for ( int i=0; i<m_wInQueu; i++ )
	{
		m_szAryInData[i] = new char[m_dwQueuBufferSize];
		m_pAryHdr[i] = new WAVEHDR;
		if ( !m_szAryInData[i] || !m_pAryHdr[i] )
		{
			::AfxThrowMemoryException ();
			return FALSE;
		}
		memset ( m_szAryInData[i], 0, m_dwQueuBufferSize );
		memset ( m_pAryHdr[i], 0, sizeof(WAVEHDR) );
	}
	
	return TRUE;
}

BOOL CAudioCtrl::AddInputBufferToQueue(int nIndex)
{
	ASSERT ( nIndex >= 0 && nIndex < m_wInQueu );
	ASSERT ( m_szAryInData[nIndex] );
	MMRESULT mmReturn = 0;
	
	LPWAVEHDR pHdr = m_pAryHdr[nIndex];
	ZeroMemory ( pHdr, sizeof(WAVEHDR) );
	pHdr->lpData = (char*)m_szAryInData[nIndex];
	pHdr->dwBufferLength = m_dwQueuBufferSize;
	
	// prepare it
	mmReturn = ::waveInPrepareHeader ( m_hRecord, pHdr, sizeof(WAVEHDR) );
	if ( mmReturn )
	{
		TRACE("waveInPrepareHeader Failed\r\n");
		return FALSE;
	}
	
	// add the input buffer to the queue
	mmReturn = ::waveInAddBuffer ( m_hRecord, pHdr, sizeof(WAVEHDR) );
	if ( mmReturn )
	{
		TRACE("waveInAddBuffer() failed\r\n");
		return FALSE;
	}
	
	m_nDataQueueNum ++;

	return TRUE;
}

BOOL CAudioCtrl::SetRecordAudioFile(UINT uiChancel, char *pszFileName)
{
	if (uiChancel > ENUM_FILE_CHANNEL_NUM)
	{
		return FALSE;
	}

	if (m_hWaveFile[uiChancel])
		return TRUE;

	WAVEFORMATEX wfx = m_Format;
	if (uiChancel==ENUM_REC_CHANNEL_ALONE )
		wfx.nChannels = 1;

	m_hWaveFile[uiChancel] = ::mmioOpen((LPTSTR)pszFileName, NULL, MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE|MMIO_ALLOCBUF);
	if ( m_hWaveFile[uiChancel] == NULL ) 
	{
		AfxMessageBox ( "Open wave file failed" );
		return FALSE;
	}

	ZeroMemory ( &m_MMCKInfoParent[uiChancel], sizeof(MMCKINFO) );
	m_MMCKInfoParent[uiChancel].fccType = mmioFOURCC('W','A','V','E');

    MMRESULT mmResult = ::mmioCreateChunk( m_hWaveFile[uiChancel], &m_MMCKInfoParent[uiChancel], MMIO_CREATERIFF);

	ZeroMemory ( &m_MMCKInfoChild[uiChancel], sizeof(MMCKINFO) );
	m_MMCKInfoChild[uiChancel].ckid = mmioFOURCC('f','m','t',' ');
	m_MMCKInfoChild[uiChancel].cksize = sizeof(WAVEFORMATEX) + wfx.cbSize;

	mmResult = ::mmioCreateChunk(m_hWaveFile[uiChancel], &m_MMCKInfoChild[uiChancel], 0);
	mmResult = ::mmioWrite(m_hWaveFile[uiChancel], (char*)&wfx, sizeof(WAVEFORMATEX) + wfx.cbSize); 
	mmResult = ::mmioAscend(m_hWaveFile[uiChancel], &m_MMCKInfoChild[uiChancel], 0);
	m_MMCKInfoChild[uiChancel].ckid = mmioFOURCC('d', 'a', 't', 'a');
	mmResult = ::mmioCreateChunk ( m_hWaveFile[uiChancel], &m_MMCKInfoChild[uiChancel], 0 );

	return TRUE;
}

void CAudioCtrl::StopRecordAudioFile(ENUM_FILE_CHANNEL eFileChannel, CString csStopFileType)
{
	if ( csStopFileType=="wav" && m_hWaveFile[eFileChannel] )
	{
		::mmioAscend ( m_hWaveFile[eFileChannel], &m_MMCKInfoChild[eFileChannel], 0 );
		::mmioAscend ( m_hWaveFile[eFileChannel], &m_MMCKInfoParent[eFileChannel], 0 );
		::mmioClose ( m_hWaveFile[eFileChannel], 0 );
		m_hWaveFile[eFileChannel] = NULL;
	}
}

void CAudioCtrl::OnTimer(UINT nIDEvent) 
{
	switch ( nIDEvent )
	{
	case TIMER_EVENT_STOPREC:
		KillTimer ( nIDEvent );
		StopRec ();
		break;
	case TIMER_EVENT_STOPPLAY:
		KillTimer ( nIDEvent );
		StopPlay ();
		break;
	}	
	CWnd::OnTimer(nIDEvent);
}

DWORD CAudioCtrl::SetMixerDevs(UINT uiLine, DWORD* pdwMinVolume, DWORD* pdwMaxVolume)
{
	m_uiMixerID = uiLine;
	CVolumeCtrl volumeCtrl(uiLine, m_uiDeviceID);
	if (pdwMinVolume)
	{
		*pdwMinVolume = volumeCtrl.GetMinimalVolume();
	}
	if (pdwMaxVolume)
	{
		*pdwMaxVolume = volumeCtrl.GetMaximalVolume();
	}
	volumeCtrl.Enable();
	return volumeCtrl.GetCurrentVolume();
}

void CAudioCtrl::SetCurVolume(DWORD dwValue)
{
	CVolumeCtrl volumeCtrl(m_uiMixerID, m_uiDeviceID);
	volumeCtrl.SetCurrentVolume(dwValue);
}

BOOL CAudioCtrl::Play(char *pszPath)
{
	if (pszPath == NULL || m_eStatus == ENUM_STATUS_PLAYING)
	{
		return FALSE;
	}

	m_bRecording = FALSE;
	if (m_eStatus != ENUM_STATUS_READY)
	{
		return FALSE;
	}

	if (!OpenWaveFile(pszPath))
		return FALSE;

	MMRESULT mmReturn = 0;
	// open wavein device
	mmReturn = ::waveOutOpen (&m_hPlay, m_uiDeviceID, &m_Format, (DWORD)GetSafeHwnd(), NULL, CALLBACK_WINDOW);
	if ( mmReturn )
	{
		TRACE("waveOutOpen() failed\r\n");
		return FALSE;
	}
	else
	{
		// make several input buffers and add them to the input queue
		for(int i=0; i<m_wInQueu; i++)
		{
			int nSize = m_dwQueuBufferSize;
			if ( !ReadSoundDataFromFile ( m_szAryInData[i], nSize ) )
			{
				if ( i == 0 )
				{
					AfxMessageBox ( "Read sound data from file failed" );
					return FALSE;
				}
			}
			if ( !AddOutputBufferToQueue ( i, nSize ) )
				return FALSE;
		}
	}
	
	m_eStatus = ENUM_STATUS_PLAYING;
	return TRUE;
}

BOOL CAudioCtrl::OpenWaveFile(char *lpszWaveFileName)
{
	if ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] ) return FALSE; 
	
	m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] = ::mmioOpen ( (LPTSTR)lpszWaveFileName,NULL,MMIO_READ );
	if ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] == NULL ) 
	{
		AfxMessageBox ( "Open wave file failed" );
		return FALSE;
	}
	
	m_MMCKInfoParent[ENUM_FILE_CHANNEL_COMMON].fccType = mmioFOURCC('W','A','V','E');
	MMRESULT mmResult = ::mmioDescend(m_hWaveFile[ENUM_FILE_CHANNEL_COMMON], &m_MMCKInfoParent[ENUM_FILE_CHANNEL_COMMON],NULL,MMIO_FINDRIFF);
	if(mmResult)
	{
		AfxMessageBox("Error descending into file");
		::mmioClose(m_hWaveFile[ENUM_FILE_CHANNEL_COMMON],0);
		m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] = NULL;
		return FALSE;
	}
	m_MMCKInfoChild[ENUM_FILE_CHANNEL_COMMON].ckid = mmioFOURCC('f','m','t',' ');
	mmResult = mmioDescend(m_hWaveFile[ENUM_FILE_CHANNEL_COMMON],&m_MMCKInfoChild[ENUM_FILE_CHANNEL_COMMON],&m_MMCKInfoParent[ENUM_FILE_CHANNEL_COMMON],MMIO_FINDCHUNK);
	if(mmResult)
	{
		AfxMessageBox("Error descending in wave file");
		mmioClose(m_hWaveFile[ENUM_FILE_CHANNEL_COMMON],0);
		m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] = NULL;
		return FALSE;
	}
	
	DWORD bytesRead = mmioRead ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON],(LPSTR)&m_Format, m_MMCKInfoChild[ENUM_FILE_CHANNEL_COMMON].cksize );
	if ( bytesRead < 0 )
	{
		AfxMessageBox ( "Error reading PCM wave format record" );
		mmioClose ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON], 0 );
		return FALSE;
	}
	if ( !SetRelateParaAfterGetWaveFormat () )
		return FALSE;
	
	// open output sound file
	mmResult = mmioAscend ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON], &m_MMCKInfoChild[ENUM_FILE_CHANNEL_COMMON], 0 );
	if ( mmResult )
	{
		AfxMessageBox ( "Error ascending in File" );
		mmioClose ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON], 0 );
		m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] = NULL;
		return FALSE;
	}
	m_MMCKInfoChild[ENUM_FILE_CHANNEL_COMMON].ckid = mmioFOURCC('d','a','t','a');
	mmResult = mmioDescend ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON], &m_MMCKInfoChild[ENUM_FILE_CHANNEL_COMMON], &m_MMCKInfoParent[ENUM_FILE_CHANNEL_COMMON], MMIO_FINDCHUNK );
	if ( mmResult )
	{
		AfxMessageBox("error reading data chunk");
		mmioClose(m_hWaveFile[ENUM_FILE_CHANNEL_COMMON],0);
		m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] = NULL;
		return FALSE;
	}
	
	return TRUE;
}

BOOL CAudioCtrl::ReadSoundDataFromFile(LPVOID data, int &size)
{
	ASSERT ( data && size > 0 );
	ASSERT ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] );
	return ( ( size = ::mmioRead ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON], (char*)data, size) ) > 0 );
}

BOOL CAudioCtrl::AddOutputBufferToQueue(int nIndex, int nSize)
{
	ASSERT ( nIndex >= 0 && nIndex < m_wInQueu );
	ASSERT ( m_szAryInData[nIndex] );
	
	MMRESULT mmReturn = 0;
	
	// create the header
	LPWAVEHDR pHdr = m_pAryHdr[nIndex];
	memset ( pHdr, 0, sizeof(WAVEHDR) );
	
	// new a buffer
	pHdr->lpData = (char*)m_szAryInData[nIndex];
	pHdr->dwBufferLength = m_dwQueuBufferSize;
	pHdr->dwBytesRecorded = nSize;
	pHdr->dwFlags = 0;
	pHdr->dwUser = nIndex;
	
	// prepare it
	mmReturn = ::waveOutPrepareHeader ( m_hPlay, pHdr, sizeof(WAVEHDR) );
	if ( mmReturn )
	{
		TRACE("waveOutPrepareHeader() failed!\r\n");
		return FALSE;
	}
	// write the buffer to output queue
	mmReturn = ::waveOutWrite ( m_hPlay, pHdr, sizeof(WAVEHDR) );
	if (mmReturn) 
	{
		TRACE("waveOutWrite() failed!\r\n");
	}
	// increment the number of waiting buffers
	m_nDataQueueNum++;
    // 将左右声道的音频数据拷贝到缓冲区用于波形绘制
	int nBytesPickup = PickupMonoData ( m_Format.wBitsPerSample, pHdr->lpData, pHdr->dwBufferLength);

	//绘制波形
	DrawWave((DWORD)nBytesPickup);

	return TRUE;
}

void CAudioCtrl::StopPlay()
{
	MMRESULT mmReturn = 0;
	if ( !m_hPlay ) return;
	if ( m_eStatus != ENUM_STATUS_PLAYING && m_eStatus != ENUM_STATUS_STOPING )
		return;
	
	waveOutPause(m_hPlay);
	mmReturn = ::waveOutClose(m_hPlay);
	if (mmReturn)
	{
		TRACE("waveOutClose() failed!\r\n");
	}

	m_hPlay = NULL;
	if (m_hWaveFile[ENUM_FILE_CHANNEL_COMMON])
	{
		::mmioClose ( m_hWaveFile[ENUM_FILE_CHANNEL_COMMON], 0 );
		m_hWaveFile[ENUM_FILE_CHANNEL_COMMON] = NULL;
	}
	m_eStatus = ENUM_STATUS_READY;

	StopAndFreeAll();
}

void CAudioCtrl::StopAndFreeAll()
{
	if (m_hRecord)
	{
		StopRec();
	}
	
	if (m_hPlay)
	{
		StopPlay();
	}

	FreeBuffer();
}


//
// 绘图
//
BOOL CAudioCtrl::OnEraseBkgnd(CDC* pDC) 
{
	DrawBackground ( pDC );	
	return CWnd::OnEraseBkgnd(pDC);
}

void CAudioCtrl::DrawBackground(CDC *pDC, BOOL bLeftChannel)
{
	ASSERT ( pDC );
	if ( !m_brsBkGnd.GetSafeHandle() ) return;

	// 画背景
	CString csCaption;
	CRect rcBK = GetRectByChannel ( bLeftChannel );
	if ( bLeftChannel )
	{
		csCaption = "Left";
	}
	else
	{
		csCaption = "Right";
	}
	pDC->FillRect ( &rcBK, &m_brsBkGnd );

	// 画文字
	CFont *pOldFnt = NULL;
	if ( m_fntChannelText.GetSafeHandle() )
	{
		pOldFnt = pDC->SelectObject ( &m_fntChannelText );
	}
	CRect rcText = rcBK;
	rcText.DeflateRect ( 2, 2 );
	pDC->SetTextColor ( RGB(255,255,0) );
	pDC->DrawText ( csCaption, &rcText, DT_LEFT | DT_TOP | DT_SINGLELINE );
	if ( pOldFnt ) pDC->SelectObject ( pOldFnt );
}

void CAudioCtrl::DrawBackground(CDC *pDC)
{
	pDC->SetBkMode ( TRANSPARENT );
	DrawBackground ( pDC, TRUE );
	DrawBackground ( pDC, FALSE );
	// 画分隔线
	CPen *pOldPen = NULL;
	if ( m_PenPartLine.GetSafeHandle() )
		pOldPen = pDC->SelectObject ( &m_PenPartLine );
	pDC->MoveTo ( 0, m_rcClient.Height()/2 );
	pDC->LineTo ( m_rcClient.right, m_rcClient.Height()/2 );
	if ( pOldPen ) pDC->SelectObject ( pOldPen );

	// 画声卡名文字
	/*
	CFont *pOldFnt = NULL;
	CRect rcText = m_rcClient;
	rcText.DeflateRect ( 4, 4 );
	if ( m_fntDeviceNameText.GetSafeHandle() )
	{
		pOldFnt = pDC->SelectObject ( &m_fntDeviceNameText );
	}
	pDC->SetTextColor ( RGB(128,255,255) );
	pDC->DrawText ( "haha!", &rcText, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE );
	if ( pOldFnt ) pDC->SelectObject ( pOldFnt );
	*/

	// 画外框
	pDC->Draw3dRect ( &m_rcClient, COLOR_FRAME, COLOR_FRAME );
}

CRect CAudioCtrl::GetRectByChannel(BOOL bLeftChannel)
{
	CRect rcBK = m_rcClient;
	if ( bLeftChannel )
	{
		rcBK.bottom = (m_rcClient.Height() - PARTLINE_HEIGHT) / 2;
	}
	else
	{
		rcBK.top = (m_rcClient.Height() + PARTLINE_HEIGHT) / 2;
	}
	return rcBK;
}

void CAudioCtrl::SetBkColor(COLORREF clr)
{
	m_clrBK = clr;
	if ( m_brsBkGnd.GetSafeHandle() )
	{
		m_brsBkGnd.DeleteTempMap();
		m_brsBkGnd.DeleteObject();
	}
	m_brsBkGnd.CreateSolidBrush ( clr );
	if ( ::IsWindow ( m_hWnd ) )
		Invalidate ();

	if ( m_PenB.GetSafeHandle() )
		m_PenB.DeleteObject ();
	m_PenB.CreatePen ( PS_SOLID, 0, m_clrBK );
}

void CAudioCtrl::DrawWave(DWORD dwChannelBytes)
{
	CClientDC dc(this);

	BOOL bRecLeft = TRUE, bRecRight = TRUE;
	if ( !m_bAlwaysDrawTowChannel && m_eStatus == ENUM_STATUS_RECORDING && m_eRecChannel == ENUM_REC_CHANNEL_ALONE )
	{
		if ( !m_hWaveFile[ENUM_FILE_CHANNEL_LEFT])
			bRecLeft = FALSE;
		if ( !m_hWaveFile[ENUM_FILE_CHANNEL_RIGHT])
			bRecRight = FALSE;
	}

	// 先将上次画的擦掉
	DrawBackground ( &dc );
	// 画波形图
	CPen *pOndPen = dc.SelectObject ( &m_PenG );
	if (m_Format.wBitsPerSample == 8)
	{
		if (bRecLeft) 
		{
			DrwaWaveChar(dc, dwChannelBytes/2, (BYTE*)m_szLeftInData, TRUE);
		}

		if (bRecRight) 
		{
			DrwaWaveChar(dc, dwChannelBytes/2, (BYTE*)m_szRightInData, FALSE);
		}
	}
	else
	{
		if (bRecLeft)
		{
			DrwaWaveShort(dc, dwChannelBytes/2, (SHORT*)m_szLeftInData, TRUE);
		}

		if (bRecRight) 
		{
			DrwaWaveShort(dc, dwChannelBytes/2, (SHORT*)m_szRightInData, FALSE);
		}
	}

	if (pOndPen)
		dc.SelectObject ( pOndPen );
}

void CAudioCtrl::DrwaWaveChar(CClientDC &dc, DWORD dwDrawBytes, BYTE *pCharData, BOOL bLeftChannel)
{
	CRect rcBK = GetRectByChannel ( bLeftChannel );
	int y = (int) ( pCharData[0] * rcBK.Height() / 0xff );
	dc.MoveTo ( 0, y );
	float fStep = (float)rcBK.Width() / (float)(dwDrawBytes);
	float fLineX = 0;
	for ( DWORD i=1; i<dwDrawBytes; i++ )
	{
		fLineX += fStep;
		y = (int) ( pCharData[i] * rcBK.Height() / 0xff );
		dc.LineTo ( (int)fLineX, y );
		//TRACE2("DrawWave X=%f Y=%d\r\n", fLineX, y);
	}
}

void CAudioCtrl::DrwaWaveShort(CClientDC &dc, DWORD dwDrawBytes, SHORT *pShortData, BOOL bLeftChannel)
{
	CRect rcBK = GetRectByChannel ( bLeftChannel );
	int nCenterY = rcBK.CenterPoint().y;
	int y = nCenterY + (int) ( pShortData[0] * rcBK.Height() / 0xffff );
	dc.MoveTo ( 0, y );
	float fStep = (float)rcBK.Width() / (float)(dwDrawBytes);
	float fLineX = 0;
	for ( DWORD i=1; i<dwDrawBytes; i++ )
	{
		fLineX += fStep;
		y = nCenterY + (int) ( pShortData[i] * rcBK.Height() / 0xffff );
		dc.LineTo ( (int)fLineX, y );

		//TRACE2("DrwaWaveShort X=%f Y=%d\r\n", fLineX, y);
	}
}

BOOL CAudioCtrl::IsPlay()
{
	if (m_eStatus == ENUM_STATUS_PLAYING)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CAudioCtrl::IsRecord()
{
	if (m_eStatus == ENUM_STATUS_RECORDING)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CAudioCtrl::SetPlayVolume(DWORD dwVolume)
{
    if (m_eStatus == ENUM_STATUS_PLAYING)
    {
        if(waveOutSetVolume( m_hPlay, dwVolume ) == MMSYSERR_NOERROR)
            return TRUE;
	}
    return FALSE;
}