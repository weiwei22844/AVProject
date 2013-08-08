#ifndef F264DEC_API_H
#define F264DEC_API_H

#ifdef __cplusplus 
extern "C" { 
#endif

typedef enum dec_status
{

	DEC_ERROR =	-1,
	DEC_SUCCESS	,
	DEC_NEED_MORE_BITS,
	DEC_NO_PICTURE,
	DEC_BUF_OVERFLOW
}DEC_STATUS;

typedef void* F264_DEC;

typedef struct f264_frame
{
	int		iPicFlag;		//0: Frame, 1: Filed
	int		iKeyFlag;		//0: Not key frame, 1:Key frame
	int		iWidth;			//The width of output picture in pixel
	int		iHeight;		//The height of output picture in pixel
	int		iStrideY;		//Luma plane stride in pixel
	int		iStrideUV;		//Chroma plane stride in pixel
	
	unsigned char *pY;		//Y plane base address of the picture
	unsigned char *pU;		//U plane base address of the picture
	unsigned char *pV;		//V plane base address of the picture
}F264_FRAME;


/*********************************************************************
* Function Name  : F264DecLibVer
* Description    : display f264 decoder lib version
* Parameters     : none 
* Return Type    : none
*********************************************************************/
void F264DecLibVer();


/*********************************************************************
* Function Name  : F264Dec_Init
* Description    : init h264 decoder module, should be called before 
*				   any operation, and only be called once!
* Parameters     : none 
* Return Type    : F264DEC_SUCCESS, success
*                  otherwise, error
*********************************************************************/
int F264DecInit();


/*********************************************************************
* Function Name  : F264Dec_Open
* Description    : open a decoder handle for decoding stream
* Parameters     : none 
* Return Type    : handle, success
*                  NULL, error
*********************************************************************/
F264_DEC F264DecOpen();


/*********************************************************************
* Function Name  : F264Dec_DecodeFrame
* Description    : decode input stream
* Parameters     : hHandle, decoder handle
*                : pBuf, point to input data if you have
*                : uBufLen, input data length
*                : pFrame, point to frame info return from decoder
*                : iFlag, set to 1 if no more data, otherwise set to 0
* Return Type    : F264DEC_SUCCESS, one frame has been decode 
*                : F264DEC_NEED_MORE_BITS, need more data to decode
*				 : F264DEC_NO_PICTURE, no more data in decoder input buffer
*				 : F264DEC_BUF_OVERFLOW, decoder buffer can't hold too much input data
*                : otherwise, error
*********************************************************************/
int F264DecFrame(F264_DEC hHandle,unsigned char *pBuf,unsigned int uBufLen,F264_FRAME *pFrame,int iFlag);


/*********************************************************************
* Function Name  : F264Dec_Close
* Description    : close decoder
* Parameters     : hHandle, decoder handle 
* Return Type    : F264DEC_SUCCESS, success
*                  otherwise, error
*********************************************************************/
int F264DecClose(F264_DEC hHandle);


/*********************************************************************
* Function Name  : F264Dec_Release
* Description    : close h264 decoder module
* Parameters     : none
* Return Type    : don't care
*********************************************************************/
int F264DecRelease();


#ifdef __cplusplus 
} 
#endif

#endif //API_H
