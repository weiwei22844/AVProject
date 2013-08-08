#ifndef DDRENDER_H
#define DDRENDER_H

#include "videorender.h"
#include "colorspace.h"

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>

// #define USE_OSD_SURFACE			// delete the comment by ZWW for testing 2011/07/19
#define LPMYDIRECTDRAWSURFACE	LPDIRECTDRAWSURFACE7

class DDOffscreenRender : public VideoRender
{
public:
	// ���캯��
	DDOffscreenRender();
	// ��������
	virtual ~DDOffscreenRender();
	// �ӿں���
	virtual int init(int index, HWND hWnd, int width, int height, draw_callback cb);	

	virtual int render(unsigned char *py, unsigned char *pu, unsigned char *pv, 
		int width, int height,RECT*rc);	

	virtual void resize();

	virtual int clean();

	virtual void setparam(int type, void * param);

private:
	// ��ʼ��DirectDraw
	int initDirectDraw();

	// ������ͼ����
	HRESULT createDrawSurface();

	// �ڲ�����
	HRESULT destroyDDObjects();

	// ����Ƿ�֧��FourCC (���ַ�����)
	BOOL hasFourCCSupport(LPDIRECTDRAWSURFACE7 lpdds);

	void adjustRect(RECT &rcSrc, RECT &rcDest);

private:
	// �����Ĳ���
	int	m_index;
	int	m_width;	// Ҫ�����Ļ�ͼ����Ŀ��
	int	m_height;	// Ҫ�����Ļ�ͼ����ĸ߶�
	// ���ھ��
	HWND	m_hWnd;
	draw_callback m_callback;

private: // DirectDraw ���
	// DirectDraw����
	LPDIRECTDRAW7        m_pDD;						// DirectDraw����
	LPDIRECTDRAWSURFACE7 m_pDDSPrimary;				// ������
	LPDIRECTDRAWSURFACE7 m_pDDSVideoSurface;		// ��ͼ����

#ifdef USE_OSD_SURFACE
	LPMYDIRECTDRAWSURFACE m_pDDRGBSurface;           //����OSD����
#endif
	// ��ͼ������Ϣ
	DDCAPS  m_ddCaps;

	// FourCC֧�ֱ�ʶ
	BOOL m_hasFourCCSupport;

	color_convert_func m_colorConvert;

	RECT m_destRect;
	RECT m_destRectNew;//ʵ��ˢ��������
	//��Ļ��͸�
	int	m_screenWidth;
	int	m_screenHeight;

	int m_MonitorBeginPosX;
	int m_MonitorBeginPosY;
	
	bool m_bSync;//��ʾ����Ƶͬ��
	bool m_bRenderOverflow;//�Ƿ�Խ���ڱ߽�
	RECT m_rtRenderOverflow_Rect;//���ڸ����߽����Ӧԭʼ���ڱ߽��ƫ��,������Ϊ��,����Ϊ��
	bool m_bRePaintBack;//���»滭����

	bool m_bCorWHRateOrg;//��͸߱���ԭʼ����

private:
	bool GetMonitorBeginPos(int* width, int*height);
};

#endif /* DDRENDER_H */
