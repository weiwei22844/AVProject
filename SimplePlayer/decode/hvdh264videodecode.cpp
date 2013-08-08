#include "stdafx.h"
#include "hvdh264videodecode.h"
#include "../utils.h"
#include <windows.h>

#define MINQP 25
#define MAXQP 31

CritSec g_vdh264_critsec;

H_F264DecLibVer HVDH264VideoDecoder::m_Fun_H264_Dec_Ver = NULL;
H_F264DecInit	HVDH264VideoDecoder::m_Fun_H264_Dec_Init = NULL;
H_F264DecOpen	HVDH264VideoDecoder::m_Fun_H264_Dec_Open = NULL;
H_F264DecFrame	HVDH264VideoDecoder::m_Fun_H264_Dec_Frame = NULL;
H_F264DecClose	HVDH264VideoDecoder::m_Fun_H264_Dec_Close = NULL;
H_F264DecRelease HVDH264VideoDecoder::m_Fun_H264_Dec_Release = NULL;
	
HVDH264VideoDecoder::HVDH264VideoDecoder()
{
	m_width = 0;
	m_height = 0;
	m_preBuffer = 0;
	//luhj add
	m_hDecOpen = 0;
}

HVDH264VideoDecoder::~HVDH264VideoDecoder()
{
    clean();
}

static int GetModulePathLen(char* cFileName)
{
	int pos = 0;

	for (;pos < 1024; pos++)
	{
		//if (memcmp(cFileName+pos, "hikplaympeg4.dll", 10) == 0)
		if (memcmp(cFileName+pos, "vnplayer.dll", 12) == 0)
		{
			break;
		}
	}

	if (pos == 1024)
	{
		pos = -1;
	}

	return pos;
}

static char cFileName[256+1024];

int HVDH264VideoDecoder::init()
{
	if (1)
	{
		g_vdh264_critsec.Lock();

		static int g_initall = 0;
		if (g_initall == 0)
		{
			HINSTANCE hInst;
			hInst = LoadLibrary("f264.dll");
			m_Fun_H264_Dec_Init = (H_F264DecInit)GetProcAddress(hInst,"F264DecInit");
			m_Fun_H264_Dec_Ver = (H_F264DecLibVer)GetProcAddress(hInst,"F264DecLibVer");
			m_Fun_H264_Dec_Open = (H_F264DecOpen)GetProcAddress(hInst,"F264DecOpen");
			m_Fun_H264_Dec_Frame = (H_F264DecFrame)GetProcAddress(hInst,"F264DecFrame");
			m_Fun_H264_Dec_Close = (H_F264DecClose)GetProcAddress(hInst,"F264DecClose");
			m_Fun_H264_Dec_Release = (H_F264DecRelease)GetProcAddress(hInst,"F264DecRelease");
			if (NULL != m_Fun_H264_Dec_Init)
			{
				m_Fun_H264_Dec_Init();					//这里初始化解码库
			}

			g_initall = 1;
		}
		g_vdh264_critsec.UnLock();
		if (NULL == m_Fun_H264_Dec_Init || NULL == m_Fun_H264_Dec_Open ||
			NULL == m_Fun_H264_Dec_Frame || NULL == m_Fun_H264_Dec_Close || NULL == m_Fun_H264_Dec_Release)
		{
			return -1;
		}
	}
	if (m_hDecOpen == 0)
	{
		__try
		{
			if (NULL != m_Fun_H264_Dec_Open)
			{
				m_hDecOpen = m_Fun_H264_Dec_Open();
			}
		} 
		__except (EXCEPTION_EXECUTE_HANDLER) 
		{
			switch(GetExceptionCode())
			{
				case EXCEPTION_ACCESS_VIOLATION:
					//OutputDebugString("Exception: h264_decode_init access violation!\n");
					return -1;
				default:
					//OutputDebugString("Exception: h264_decode_init unknown exception!\n");
					return -1;		
			}		
		}
		if (m_hDecOpen == 0) 
		{
			return -1;
		}
	}
	//pVideoFile = fopen("C:\\Video.YUV","ab");
	return 0;
}

static inline void swapimg2(unsigned char *src, unsigned char *dst, int width, int height, int flag)
{
	int i = 0;
	for (i = 0; i < height; i++)
	{
		memcpy(dst+(2*i+flag)*width, src+i*width, width);
	}

	for (i = 0; i < height/2; i++)
	{
		memcpy(dst+height*2*width + (2*i+flag)*width/2, src+height*width + i*width/2, width/2);
	}

	for (i = 0; i < height/2; i++)
	{
		memcpy(dst+height*width*5/2 + (2*i+flag)*width/2, src+height*width*5/4 + i*width/2, width/2);
	}
}

int HVDH264VideoDecoder::clean()
{
	if (m_hDecOpen == 0) 
	{
		return 0;
	}

	__try
	{
		//fclose(pVideoFile);
		m_Fun_H264_Dec_Close(m_hDecOpen);
		m_Fun_H264_Dec_Release();
		m_hDecOpen = 0;
	} 
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		switch(GetExceptionCode())
		{
			case EXCEPTION_ACCESS_VIOLATION:
				//OutputDebugString("Exception: h264_decode_end access violation!\n");
				return -1;
			default:
				//OutputDebugString("Exception: h264_decode_end unknown exception!\n");
				return -1;		
		}		
	}
	
    return 0;
}

int HVDH264VideoDecoder::resize(int w, int h)
{
 	if (m_width != w || m_height != h)
	{
		m_width  = w;
		m_height = h;
		reset();
	}

	return 0 ;
}

// 解码H264码流
int HVDH264VideoDecoder::decode(unsigned char *buf, unsigned int buflen,unsigned char* dest_buf, int param1, int param2)
{
	if(m_hDecOpen == 0)
		return -1;
	
	int error = 0;
	int flag = 0;
	__try 
	{	
		int iDecReturn=0;
		int inite = error;
		unsigned char* ptr = buf;
		int decLen = buflen;
		//兼容索尼设备，过滤掉sei 
		if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1 && buf[4] == 6)
		{
			unsigned int code = 0xffffffff;
			for (unsigned int i = 0; i < buflen - 4; i++)
			{
				code = code << 8 | ptr[i];
				if (code == 0x01 && ((ptr[i+1] & 0x0f) == 0x01))
				{
					ptr = buf + i - 3;
					decLen = buflen - i + 3;
					break;
				}
			}
		}

		error = m_Fun_H264_Dec_Frame(m_hDecOpen,ptr,decLen,&m_TempBuffer,2);
		if (DEC_SUCCESS == error)
		{
			error = buflen;
            m_width = m_TempBuffer.iWidth;
            m_height = m_TempBuffer.iHeight;

			int i = 0;
			for(i=0;i<m_TempBuffer.iHeight;i++)
			{
				memcpy(dest_buf+ i*m_TempBuffer.iWidth, m_TempBuffer.pY + i*m_TempBuffer.iStrideY,m_TempBuffer.iWidth);
			}
			for(i=0;i<m_TempBuffer.iHeight/2;i++)
			{
				memcpy(dest_buf+ m_TempBuffer.iWidth*m_TempBuffer.iHeight+i*m_TempBuffer.iWidth/2,
					m_TempBuffer.pU + i*m_TempBuffer.iStrideUV,
					m_TempBuffer.iWidth/2);
			}
  			for(i=0;i<m_TempBuffer.iHeight/2;i++)
  			{
  				memcpy(dest_buf+ m_TempBuffer.iWidth*m_TempBuffer.iHeight*5/4+i*m_TempBuffer.iWidth/2,
  					m_TempBuffer.pV + i*m_TempBuffer.iStrideUV,
  					m_TempBuffer.iWidth/2);
  			}			
		}
		else
		{
			error = -1;	
		}
	} 
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		switch(GetExceptionCode())
		{
			case EXCEPTION_ACCESS_VIOLATION:
				//OutputDebugString("Exception: h264_decode_frame access violation!\n");
				error = -1;
				break;
			default:
				//OutputDebugString("Exception: h264_decode_frame unknown exception!\n");
				error = -1;
				break;			
 		}	
	}

	return error;
}

int HVDH264VideoDecoder::reset()
{
	int ret = clean();
	if (ret < 0) 
	{
		return ret; 
	}
		
	ret = init();
	if (ret < 0) 
	{
		m_hDecOpen = 0;
	}

	return ret;
}

bool HVDH264VideoDecoder::Getsolu(int *width, int *height)
{
	*width = m_width;
	*height = m_height;
	return true;
}
