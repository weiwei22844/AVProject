// SampleGrabberCB.h: interface for the CSampleGrabberCB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAMPLEGRABBERCB_H__06441262_12E6_4066_AA37_4A1127F89AD9__INCLUDED_)
#define AFX_SAMPLEGRABBERCB_H__06441262_12E6_4066_AA37_4A1127F89AD9__INCLUDED_
#include "SimplePlayerDlg.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Structures
typedef struct _callbackinfo 
{
    double dblSampleTime;
    long lBufferSize;
    BYTE *pBuffer;
    BITMAPINFOHEADER bih;
} CALLBACKINFO;

class CSampleGrabberCB : public ISampleGrabberCB  
{
public:
    // these will get set by the main thread below. We need to
    // know this in order to write out the bmp
    long lWidth;
    long lHeight;
    CSimplePlayerDlg * pOwner;
    TCHAR m_szCapDir[MAX_PATH]; // the directory we want to capture to
    TCHAR m_szSnappedName[MAX_PATH];
    BOOL bFileWritten;

public:
	CSampleGrabberCB();
	virtual ~CSampleGrabberCB();

    STDMETHODIMP BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize );
    BOOL DisplayCapturedBits(BYTE *pBuffer, BITMAPINFOHEADER *pbih);
    BOOL CopyBitmap( double dblSampleTime, BYTE * pBuffer, long lBufferSize );
    void PlaySnapSound(void);
    // fake out any COM ref counting
    //
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    
    // fake out any COM QI'ing
    //
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
        {
            *ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
            return NOERROR;
        }    
        return E_NOINTERFACE;
    }
    
    // we don't implement this interface for this example
    //
    STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample )
    {
        return 0;
    }

};

#endif // !defined(AFX_SAMPLEGRABBERCB_H__06441262_12E6_4066_AA37_4A1127F89AD9__INCLUDED_)
