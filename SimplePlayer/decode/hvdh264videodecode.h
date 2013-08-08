#ifndef VDH264VIDEODECODE_H
#define VDH264VIDEODECODE_H

#include <stdio.h>
#include "interface.h"
#include "../depend/f264dec_api.h"

typedef	void (*H_F264DecLibVer)();
typedef int (*H_F264DecInit)();
typedef F264_DEC (*H_F264DecOpen)();
typedef int (*H_F264DecFrame)(F264_DEC hhandle, unsigned char *pBuf, unsigned int uBufLen, F264_FRAME *pFrame, int iFlag);
typedef int (*H_F264DecClose)(F264_DEC hHandle);
typedef int (*H_F264DecRelease)();    


class HVDH264VideoDecoder:public IDecode
{
public:
    HVDH264VideoDecoder();
    ~HVDH264VideoDecoder();
    
	/************************************************************************/
	/* 视频解码接口(初始化，清除，重置，图像大小发生改变，解码，etc			*/
	/************************************************************************/
    int init();		// 初始化
    int clean();	// 清除
	int reset();	// 重置
    int resize(int w, int h); // 改变图像大小，可以不用
    int decode(unsigned char *src_buf,unsigned int buflen,unsigned char* dest_buf, int param1, int param2); // 解码
	bool Getsolu(int *width, int *height);
	int* getExtInfo(){return 0;}
	
private:
	//void *m_decHandle;	
	int m_qp;
	int m_width;
	int m_height;
	unsigned char* m_preBuffer;
	F264_FRAME m_TempBuffer;
	FILE *pVideoFile;
	F264_DEC m_hDecOpen;
public:
	//	以下为高清H264解码部分
	static H_F264DecLibVer	m_Fun_H264_Dec_Ver;
	static H_F264DecInit	m_Fun_H264_Dec_Init;
	static H_F264DecOpen	m_Fun_H264_Dec_Open; 
	static H_F264DecFrame	m_Fun_H264_Dec_Frame;
	static H_F264DecClose	m_Fun_H264_Dec_Close;
	static H_F264DecRelease	m_Fun_H264_Dec_Release;
	
};

#endif /* HVDH264VIDEODECODE_H */