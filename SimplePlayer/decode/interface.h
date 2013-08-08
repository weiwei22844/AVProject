#ifndef _INTERFACE_H
#define _INTERFACE_H

struct IDecode//解码接口
{
	virtual  int decode(unsigned char *src_buf,unsigned int buflen,unsigned char* dest_buf, int param1, int param2)=0; // 解码	
	virtual  int reset() = 0 ;
	virtual  int resize(int w, int h) = 0;
	//add by HY 2009-8-13 begin
	virtual  bool Getsolu(int *width, int *height) = 0;
	//add by HY 2009-8-13 end

	virtual  int* getExtInfo() = 0;
};

#endif