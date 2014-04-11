#if !defined(AFX_AUDIOCTRL_H__17F72165_6C67_494E_B9D5_9935044316F0__INCLUDED_)
#define AFX_AUDIOCTRL_H__17F72165_6C67_494E_B9D5_9935044316F0__INCLUDED_

#include "VolumeCtrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef void (*MsgNotifyProc)(WPARAM wParam, LPARAM lParam);

class CAudioCtrl : public CWnd
{
// Construction
public:
	CAudioCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	DWORD SetMixerDevs(UINT uiLine, DWORD* pdwMinVolume=NULL, DWORD* pdwMaxVolume=NULL);
	void Stop();
	BOOL SetRecordAudioFile(UINT uiChancel,char* pszFileName);
	BOOL Record(UINT uiRecChannel, DWORD dwSamplingRate, WORD  wSamplingBit);
	UINT GetWaveOutCount();
	UINT InitCtrl(UINT uiDevIndex, DWORD dwBufSize = 1024, HWND hwndParent = NULL, LPRECT lpRect = NULL, MsgNotifyProc msgNodityProc=NULL, WPARAM wParam=NULL);

	UINT GetWaveInAvailableDev();
	char* GetWaveInName(UINT uiDevID);
	UINT GetWaveInDevs();
	virtual ~CAudioCtrl();

    BOOL SetPlayVolume(DWORD dwVolume);

	// Generated message map functions
protected:
	LRESULT OnMM_WIM_DATA (WPARAM wParam, LPARAM lParam);
	LRESULT OnMM_WOM_DONE (WPARAM wParam, LPARAM lParam);
	//{{AFX_MSG(CAudioCtrl)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL IsRecord();
	BOOL IsPlay();
	BOOL Play(char* pszPath);
	void SetCurVolume(DWORD dwValue);
    void SetDeviceID(UINT uiIndex);
	typedef enum _enum_status
	{
		ENUM_STATUS_INVALID,		// 无效的状态
		ENUM_STATUS_READY,			// 准备就绪
		ENUM_STATUS_PLAYING,		// 正在播放
		ENUM_STATUS_RECORDING,		// 正在录音
		ENUM_STATUS_STOPING,		// 正在停止
		ENUM_STATUS_NUM,
	}ENUM_STATUS;

	// 录音的声道
	typedef enum _enum_rec_channel
	{
		ENUM_REC_CHANNEL_MONO,
		ENUM_REC_CHANNEL_STEREO,
		ENUM_REC_CHANNEL_ALONE,
		ENUM_REC_CHANNEL_NUM,
	} ENUM_REC_CHANNEL;

	// 保存文件的通道
	typedef enum _enum_file_channel
	{
		ENUM_FILE_CHANNEL_COMMON,
		ENUM_FILE_CHANNEL_LEFT=ENUM_FILE_CHANNEL_COMMON,
		ENUM_FILE_CHANNEL_RIGHT,
		ENUM_FILE_CHANNEL_NUM,
	} ENUM_FILE_CHANNEL;

	enum
	{
		TIMER_EVENT_STOPREC = 1,
		TIMER_EVENT_STOPPLAY,
		TIMER_EVENT_NUM,
	};

private:
	void DrwaWaveShort(CClientDC &dc, DWORD dwDrawBytes, SHORT *pShortData, BOOL bLeftChannel);
	void DrwaWaveChar(CClientDC &dc, DWORD dwDrawBytes, BYTE *pCharData, BOOL bLeftChannel);
	void DrawWave(DWORD dwChannelBytes);
	void SetBkColor(COLORREF clr);
	CRect GetRectByChannel(BOOL bLeftChannel);
	void DrawBackground(CDC *pDC);
	void DrawBackground ( CDC *pDC, BOOL bLeftChannel );
	void StopAndFreeAll();
	void StopPlay();
	BOOL AddOutputBufferToQueue(int nIndex, int nSize);
	BOOL ReadSoundDataFromFile(LPVOID data, int &size);
	BOOL OpenWaveFile(char* lpszWaveFileName);
	void StopRecordAudioFile ( ENUM_FILE_CHANNEL eFileChannel, CString csStopFileType="wav" );
	BOOL AddInputBufferToQueue(int nIndex);
	BOOL AllocateBuffer(DWORD dwBufferSize);
	BOOL SetRelateParaAfterGetWaveFormat();
	void FreeBuffer();
	void SetWaveFormat(UINT uiRecChannel, DWORD dwSamplingRate, WORD  wSamplingBit);
	void StopRec();
	int PickupMonoData( WORD wBitsPerSample, char *szOrgData, int nOrgSize);
	BOOL Create(HWND hwndParent, LPRECT lpRect/*=NULL*/);

	//void SetDeviceID(UINT uiIndex);           // moved by ZWW

	UINT				m_uiDeviceID;
	UINT                m_uiMixerID;
	DWORD				m_dwQueuBufferSize;
	MsgNotifyProc		m_pMsgNotifyProc;
	WPARAM				m_wParam;
	ENUM_STATUS			m_eStatus;
	ENUM_REC_CHANNEL	m_eRecChannel;
	int					m_nDataQueueNum;
	BOOL				m_bRecording;

	HWAVEIN				m_hRecord; // 录制句柄
	HWAVEOUT			m_hPlay;   // 播放句柄

	WAVEFORMATEX		m_Format;

	WORD				m_wInQueu;        // 声音数据缓冲的数量
	char				**m_szAryInData;  // 声音的数据缓冲
	char				*m_szLeftInData;  // 左声道的数据缓冲
	char				*m_szRightInData; // 右声道的数据缓冲
	WAVEHDR				**m_pAryHdr;

	HMMIO				m_hWaveFile[ENUM_FILE_CHANNEL_NUM];
	MMCKINFO			m_MMCKInfoParent[ENUM_FILE_CHANNEL_NUM];
	MMCKINFO			m_MMCKInfoChild[ENUM_FILE_CHANNEL_NUM];






	CBrush				m_brsBkGnd;					// 背景刷
	COLORREF			m_clrBK;
	CRect				m_rcClient;
	CPen				m_PenB;
	CPen				m_PenG;
	CPen				m_PenPartLine;
	CFont				m_fntChannelText;
	CFont				m_fntDeviceNameText;

	BOOL m_bAlwaysDrawTowChannel;				// 总是画两个声道的声波图

	enum
	{
		PARTLINE_HEIGHT = 2,
		COLOR_FRAME = RGB(128,64,0),
	};
};

#endif // !defined(AFX_AUDIOCTRL_H__17F72165_6C67_494E_B9D5_9935044316F0__INCLUDED_)
