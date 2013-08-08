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
	// 构造函数
	DDOffscreenRender();
	// 析构函数
	virtual ~DDOffscreenRender();
	// 接口函数
	virtual int init(int index, HWND hWnd, int width, int height, draw_callback cb);	

	virtual int render(unsigned char *py, unsigned char *pu, unsigned char *pv, 
		int width, int height,RECT*rc);	

	virtual void resize();

	virtual int clean();

	virtual void setparam(int type, void * param);

private:
	// 初始化DirectDraw
	int initDirectDraw();

	// 创建绘图表面
	HRESULT createDrawSurface();

	// 内部清理
	HRESULT destroyDDObjects();

	// 检查是否支持FourCC (四字符代码)
	BOOL hasFourCCSupport(LPDIRECTDRAWSURFACE7 lpdds);

	void adjustRect(RECT &rcSrc, RECT &rcDest);

private:
	// 给定的参数
	int	m_index;
	int	m_width;	// 要创建的绘图表面的宽度
	int	m_height;	// 要创建的绘图表面的高度
	// 窗口句柄
	HWND	m_hWnd;
	draw_callback m_callback;

private: // DirectDraw 相关
	// DirectDraw对象
	LPDIRECTDRAW7        m_pDD;						// DirectDraw对象
	LPDIRECTDRAWSURFACE7 m_pDDSPrimary;				// 主表面
	LPDIRECTDRAWSURFACE7 m_pDDSVideoSurface;		// 绘图表面

#ifdef USE_OSD_SURFACE
	LPMYDIRECTDRAWSURFACE m_pDDRGBSurface;           //用于OSD叠加
#endif
	// 绘图能力信息
	DDCAPS  m_ddCaps;

	// FourCC支持标识
	BOOL m_hasFourCCSupport;

	color_convert_func m_colorConvert;

	RECT m_destRect;
	RECT m_destRectNew;//实际刷窗体区域
	//屏幕宽和高
	int	m_screenWidth;
	int	m_screenHeight;

	int m_MonitorBeginPosX;
	int m_MonitorBeginPosY;
	
	bool m_bSync;//显示器视频同步
	bool m_bRenderOverflow;//是否超越窗口边界
	RECT m_rtRenderOverflow_Rect;//窗口各个边界相对应原始窗口边界的偏移,以向右为正,向下为正
	bool m_bRePaintBack;//重新绘画背景

	bool m_bCorWHRateOrg;//宽和高保留原始比例

private:
	bool GetMonitorBeginPos(int* width, int*height);
};

#endif /* DDRENDER_H */
