#include "gdirender.h"
#include <stdio.h>

GDIRender::RGBPlane::RGBPlane()
: data(0), w(0), h(0)
{
}

GDIRender::RGBPlane::~RGBPlane()
{
    if (data) 
	{
        free(data);
    }
}

void GDIRender::RGBPlane::resize(int width, int height)
{
    if (data) 
	{
        if (w == width && height == h)
        {
 			return;
		}

		free(data);
        data = (unsigned char *) malloc( width * height * 3);
    } 
	else 
	{
        data = (unsigned char *) malloc(width * height * 3);	
    }
    
    memset(data, 0, width * height * 3);
    
    w = width;
    h = height;
}

unsigned char *GDIRender::RGBPlane::getLine(int y)
{
    return &data[y*w*3];
};

GDIRender::GDIRender()
{
	m_hWnd	= 0;
	m_cb	= 0;
	m_flip				= true;
	m_rgbReverseOrder	= true;
}

GDIRender::~GDIRender()
{
}

int GDIRender::init(int index, HWND hWnd, int width, int height, draw_callback cb)
{
	int iRt = -1;
	m_index	= index;
	m_hWnd	= hWnd;
	m_cb	= cb;
    iRt = init_dither_tab();
	if(iRt == 0)
	{
		return -1;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char *clp = NULL;
unsigned char *clp1 = NULL;
long int crv_tab[256];
long int cbu_tab[256];
long int cgu_tab[256];

long int cgv_tab[256];
long int tab_76309[256];

int GDIRender::init_dither_tab()
{
    long int crv, cbu, cgu, cgv;
    int i;
    
    crv = 104597;
    cbu = 132201;                 /* fra matrise i global.h */
    cgu = 25675;
    cgv = 53279;
    
    for (i = 0; i < 256; i++)
    {
        crv_tab[i] = (i - 128) * crv;
        cbu_tab[i] = (i - 128) * cbu;
        cgu_tab[i] = (i - 128) * cgu;
        cgv_tab[i] = (i - 128) * cgv;
        tab_76309[i] = 76309 * (i - 16);
    }
    if (!(clp = (unsigned char *)malloc(sizeof(unsigned char)*1024)))
    {
        OutputDebugString("安排解码码表内存失败！");
		return 0;
    }
    clp1 = clp;
    
    clp += 384;
    
    for (i = -384; i < 640; i++)
        clp[i] = (i < 0) ? 0 : ((i > 255) ? 255 : i);

	return 1;
}

void ConvertYuvToRGB24Bstream(LPBYTE pY, LPBYTE pCb, LPBYTE pCr,
                                            LPBYTE hpOutput, int imWidth, int imHeight)
{
    int y11, y21;
    int y12, y22;
    int y13, y23;
    int y14, y24;
    int u, v;
    int i, j;
    int c11, c21, c31, c41;
    int c12, c22, c32, c42;
    unsigned int DW;
    unsigned int *id1, *id2;
    unsigned char *py1, *py2, *pu, *pv;
    unsigned char *d1, *d2;
    
    d1 = hpOutput;
    d1 += imWidth * imHeight * 3 - imWidth * 3;
    d2 = d1 - imWidth * 3;
    
    py1 = pY;
    pu = pCb;
    pv = pCr;
    py2 = py1 + imWidth;
    
    id1 = (unsigned int *) d1;
    id2 = (unsigned int *) d2;
    
    for (j = 0; j < imHeight; j += 2)
    {
        /* line j + 0 */
        for (i = 0; i < imWidth; i += 4)
        {
            u = *pu++;
            v = *pv++;
            c11 = crv_tab[v];
            c21 = cgu_tab[u];
            c31 = cgv_tab[v];
            c41 = cbu_tab[u];
            u = *pu++;
            v = *pv++;
            c12 = crv_tab[v];
            c22 = cgu_tab[u];
            c32 = cgv_tab[v];
            c42 = cbu_tab[u];
            
            y11 = tab_76309[*py1++];  /* (255/219)*65536 */
            y12 = tab_76309[*py1++];
            y13 = tab_76309[*py1++];  /* (255/219)*65536 */
            y14 = tab_76309[*py1++];
            
            y21 = tab_76309[*py2++];
            y22 = tab_76309[*py2++];
            y23 = tab_76309[*py2++];
            y24 = tab_76309[*py2++];
            
            /* RGBR */
            DW = ((clp[(y11 + c41) >> 16])) |
                ((clp[(y11 - c21 - c31) >> 16]) << 8) |
                ((clp[(y11 + c11) >> 16]) << 16) |
                ((clp[(y12 + c41) >> 16]) << 24);
            *id1++ = DW;
            
            /* GBRG */
            DW = ((clp[(y12 - c21 - c31) >> 16])) |
                ((clp[(y12 + c11) >> 16]) << 8) |
                ((clp[(y13 + c42) >> 16]) << 16) |
                ((clp[(y13 - c22 - c32) >> 16]) << 24);
            *id1++ = DW;
            
            /* BRGB */
            DW = ((clp[(y13 + c12) >> 16])) |
                ((clp[(y14 + c42) >> 16]) << 8) |
                ((clp[(y14 - c22 - c32) >> 16]) << 16) |
                ((clp[(y14 + c12) >> 16]) << 24);
            *id1++ = DW;
            
            /* RGBR */
            DW = ((clp[(y21 + c41) >> 16])) |
                ((clp[(y21 - c21 - c31) >> 16]) << 8) |
                ((clp[(y21 + c11) >> 16]) << 16) |
                ((clp[(y22 + c41) >> 16]) << 24);
            *id2++ = DW;
            
            /* GBRG */
            DW = ((clp[(y22 - c21 - c31) >> 16])) |
                ((clp[(y22 + c11) >> 16]) << 8) |
                ((clp[(y23 + c42) >> 16]) << 16) |
                ((clp[(y23 - c22 - c32) >> 16]) << 24);
            *id2++ = DW;
            
            /* BRGB */
            DW = ((clp[(y23 + c12) >> 16])) |
                ((clp[(y24 + c42) >> 16]) << 8) |
                ((clp[(y24 - c22 - c32) >> 16]) << 16) |
                ((clp[(y24 + c12) >> 16]) << 24);
            *id2++ = DW;
        }
        id1 -= (9 * imWidth) >> 2;
        id2 -= (9 * imWidth) >> 2;
        py1 += imWidth;
        py2 += imWidth;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************
 * 1. YUV --> RGB24.
 * 2. RGB24 Select to Memory DC.
 * 3. StretchBlt.
 ***********************************************************************/
int GDIRender::render(unsigned char *py, unsigned char *pu, unsigned char *pv, int width, int height, RECT*rc)
{
	int err = 0;
	HDC hDC = 0;
	HDC hMemDC = 0 ;
	HBITMAP hBitMap = 0;
	
	// 1. YUV --> RGB24

	m_rgb.resize(width, height);

    // modified by ZWW for liuliqiao 2012/01/06, there maybe a bug in YUV_TO_RGB24
    ConvertYuvToRGB24Bstream(py, pu, pv, m_rgb.getLine(0), width, height);
	//YUV_TO_RGB24(py, width, pu, pv, width/2, m_rgb.getLine(0), width, -height, width*3);		

	//获得目的绘图窗口大小
	//如果窗口最小化了，就不做处理，节约CPU资源
	RECT rcDest;
	GetClientRect(m_hWnd, &rcDest);
	if (rcDest.left<0&&rcDest.top<0&&rcDest.right<0&&rcDest.bottom<0)
	{
		return 0; 
	}
	
	//获得窗口设备,窗口DC为目标DC
	hDC = GetWindowDC(m_hWnd);
	if( hDC == NULL )
	{
		err = -1;
		goto ret;
	}

	//建立一个和窗口设备兼容的内存设备
	hMemDC = CreateCompatibleDC(hDC);
	if( hMemDC == NULL )
	{
		ReleaseDC(m_hWnd, hDC);
		err = -1;
		goto ret;
	}

	// 创建一个跟源一样大小的位图对象, 将位图对象选进内存DC
	// 准备COPY
	
	BITMAPINFO  bitmapinfo;
    BITMAPFILEHEADER bmpHeader;
	
    bmpHeader.bfType = 'MB';
    bmpHeader.bfSize = width*height*3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpHeader.bfReserved1 = 0;
    bmpHeader.bfReserved2 = 0;
    bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapinfo.bmiHeader.biWidth = width;
    bitmapinfo.bmiHeader.biHeight = height;
    bitmapinfo.bmiHeader.biPlanes = 1;
    bitmapinfo.bmiHeader.biBitCount = 24;
    bitmapinfo.bmiHeader.biCompression = BI_RGB;
    bitmapinfo.bmiHeader.biSizeImage = width*height*3;
    bitmapinfo.bmiHeader.biXPelsPerMeter = 0;
    bitmapinfo.bmiHeader.biYPelsPerMeter = 0;
    bitmapinfo.bmiHeader.biClrUsed = 0;
    bitmapinfo.bmiHeader.biClrImportant = 0;
	
	//创建一个和窗口设备兼容的位图对象
	hBitMap = CreateCompatibleBitmap(hDC, rcDest.right - rcDest.left, rcDest.bottom - rcDest.top);
	if( hBitMap == NULL )
	{
		DeleteDC(hMemDC);
		ReleaseDC(m_hWnd, hDC);
		err = -1;
		goto ret;
	}
	
	//把位图对象选进内存设备,至于这样做的原因,可以查看MSDN关于
	//创建内存设备的帮助
	SelectObject(hMemDC, hBitMap);

	//设置内存设备的StretchBIT模式
    SetStretchBltMode(hMemDC,COLORONCOLOR);

	//把源图像StretchBlt到内存设备中(有缩放)
    StretchDIBits(hMemDC, 0, 0, rcDest.right - rcDest.left ,rcDest.bottom - rcDest.top ,
        0, 0,m_rgb.w,m_rgb.h, m_rgb.getLine(0), &bitmapinfo, DIB_RGB_COLORS, SRCCOPY); 

	int srcX, srcY, srcW, srcH;
	int dstW, dstH;

	dstW = rcDest.right-rcDest.left;
	dstH = rcDest.bottom-rcDest.top;
	
	srcX = 0;
	srcY = 0;
	srcW = dstW;
	srcH = dstH;

	StretchBlt(hDC,0,0,dstW,dstH,hMemDC,srcX,srcY,srcW,srcH,SRCCOPY);

	if (m_cb)
	{
		m_cb(m_index, hDC);
	}
	
	DeleteObject(hBitMap);
	DeleteDC(hMemDC);
    ReleaseDC(m_hWnd, hDC);

	err = 0;

ret:	
	return err;
}

int GDIRender::clean()
{
	if(clp1)
	{
		delete clp1;
		clp1 = NULL;
	}
	return 0;
}

void GDIRender::resize()
{
}

#define LIMIT(x) \
(unsigned char)(((x>0xffffff)?0xff0000:((x<=0xffff)?0:x&0xff0000))>>16)

void GDIRender::colorConvert(unsigned char *py, unsigned char *pu, unsigned char *pv, int width, int height)
{ 
	const unsigned char * yplane  = (unsigned char *)py;
	const unsigned char * uplane  = (unsigned char *)pu;
	const unsigned char * vplane  = (unsigned char *)pv;

	int hh = height;
	int ww = width;

	int x, y;
	for (y = 0; y < hh; y+=2) {
		const unsigned char * yline  = yplane + (y * ww);
		const unsigned char * yline2 = yline  + ww;
		const unsigned char * uline  = uplane + ((y >> 1) * (ww >> 1));
		const unsigned char * vline  = vplane + ((y >> 1) * (ww >> 1));
        
		int yy  = y;
		if (m_flip)
			yy = (height-1) - yy;
		unsigned char * rgb  = m_rgb.getLine(yy);
		yy = y + 1;
		if (m_flip)
			yy = (hh-1) - yy;
		
		unsigned char * rgb2 = m_rgb.getLine(yy);
		
		for (x = 0; x < ww; x += 2) {
			long Cr = *uline++ - 128;     // calculate once for 4 pixels
			long Cb = *vline++ - 128;
			long lrc = 104635 * Cb;
			long lgc = -25690 * Cr + -53294 * Cb;
			long lbc = 132278 * Cr;
			
			if (m_rgbReverseOrder) { 
				long tmp;     // exchange red component and blue component
				tmp=lrc;
				lrc=lbc;
				lbc=tmp;
			}
			
			long Y  = *yline++ - 16;      // calculate for every pixel
			if (Y < 0)
				Y = 0;

			long l  = 76310 * Y;
			long lr = l + lrc;
			long lg = l + lgc;
			long lb = l + lbc;
			
			*rgb++ = LIMIT(lr);
			*rgb++ = LIMIT(lg);
			*rgb++ = LIMIT(lb);         

			Y  = *yline++ - 16;       // calculate for every pixel
			if (Y < 0)
				Y = 0;

			l  = 76310 * Y;
			lr = l + lrc;
			lg = l + lgc;
			lb = l + lbc;
			
			*rgb++ = LIMIT(lr);
			*rgb++ = LIMIT(lg);
			*rgb++ = LIMIT(lb);         

			Y  = *yline2++ - 16;     // calculate for every pixel
			if (Y < 0)
				Y = 0;

			l  = 76310 * Y;
			lr = l + lrc;
			lg = l + lgc;
			lb = l + lbc;
			
			*rgb2++ = LIMIT(lr);
			*rgb2++ = LIMIT(lg);
			*rgb2++ = LIMIT(lb);        

			Y  = *yline2++ - 16;      // calculate for every pixel
			if (Y < 0)
				Y = 0;

			l  = 76310 * Y;
			lr = l + lrc;
			lg = l + lgc;
			lb = l + lbc;
			
			*rgb2++ = LIMIT(lr);
			*rgb2++ = LIMIT(lg);
			*rgb2++ = LIMIT(lb);        
		}
	}
}

#define MAXIMUM_Y_WIDTH 800

#define int8_t   char
#define uint8_t  unsigned char
#define int16_t  short
#define uint16_t unsigned short
#define int32_t  int
#define uint32_t unsigned int
#define int64_t  __int64
#define uint64_t unsigned __int64

/* colourspace conversion matrix values */
static uint64_t mmw_mult_Y    = 0x2568256825682568;
static uint64_t mmw_mult_U_G  = 0xf36ef36ef36ef36e;
static uint64_t mmw_mult_U_B  = 0x40cf40cf40cf40cf;
static uint64_t mmw_mult_V_R  = 0x3343334333433343;
static uint64_t mmw_mult_V_G  = 0xe5e2e5e2e5e2e5e2;


/* various masks and other constants */
static uint64_t mmb_0x10      = 0x1010101010101010;
static uint64_t mmw_0x0080    = 0x0080008000800080;
static uint64_t mmw_0x00ff    = 0x00ff00ff00ff00ff;

static uint64_t mmw_cut_red   = 0x7c007c007c007c00;
static uint64_t mmw_cut_green = 0x03e003e003e003e0;
static uint64_t mmw_cut_blue  = 0x001f001f001f001f;

/**** YUV -> RGB conversion, 24-bit output ****/
void GDIRender::YUV_TO_RGB24(unsigned char *puc_y, int stride_y, 
                unsigned char *puc_u, unsigned char *puc_v, int stride_uv, 
                unsigned char *puc_out, int width_y, int height_y,int stride_out) {

	int y, horiz_count;
	unsigned char *puc_out_remembered;
//	int stride_out = width_y * 3;

	if (height_y < 0) 
	{
		/* we are flipping our output upside-down */
		height_y  = -height_y;
		puc_y     += (height_y   - 1) * stride_y ;
		puc_u     += (height_y/2 - 1) * stride_uv;
		puc_v     += (height_y/2 - 1) * stride_uv;
		stride_y  = -stride_y;
		stride_uv = -stride_uv;
	}

	horiz_count = -(width_y >> 3);

	for (y=0; y<height_y; y++) {

		if (y == height_y-1) {
			/* this is the last output line - we need to be careful not to overrun the end of this line */
			unsigned char temp_buff[3*MAXIMUM_Y_WIDTH+1];
			puc_out_remembered = puc_out;
			puc_out = temp_buff; /* write the RGB to a temporary store */
		}

		_asm {
			push eax
			push ebx
			push ecx
			push edx
			push edi

			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
			
		horiz_loop:

			movd mm2, [ecx]
			pxor mm7, mm7

			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0

			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0

			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 

			psubusb mm0, mmb_0x10    ; mm0 -= 16

			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0

			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8

			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8

			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8

			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v

			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u

			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part

			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0

			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0

			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1

			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma

			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0

			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1

			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1

			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1

			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0

			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0

			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?

			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0

			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0

			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0

			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2

			/* 24-bit shuffle and save... */
			movd   [eax], mm0        ; eax[0] = __r0g0b0
			psrlq mm0, 32            ; mm0 = __r1g1b1

			movd  3[eax], mm0        ; eax[3] = __r1g1b1

			movd  6[eax], mm2        ; eax[6] = __r2g2b2
			

			psrlq mm2, 32            ; mm2 = __r3g3b3
	
			movd  9[eax], mm2        ; eax[9] = __r3g3b3

			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?

			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4

			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4

			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4

			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6

			/* 24-bit shuffle and save... */
			movd 12[eax], mm0        ; eax[12] = __r4g4b4
			psrlq mm0, 32            ; mm0 = __r5g5b5
			
			movd 15[eax], mm0        ; eax[15] = __r5g5b5
			add ebx, 8               ; puc_y   += 8;

			movd 18[eax], mm2        ; eax[18] = __r6g6b6
			psrlq mm2, 32            ; mm2 = __r7g7b7
			
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;

			movd 21[eax], mm2        ; eax[21] = __r7g7b7
			add eax, 24              ; puc_out += 24

			inc edi
			jne horiz_loop			

			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax

			emms
						
		}

		if (y == height_y-1) {
			/* last line of output - we have used the temp_buff and need to copy... */
			int x = 3 * width_y;                  /* interation counter */
			unsigned char *ps = puc_out;                /* source pointer (temporary line store) */
			unsigned char *pd = puc_out_remembered;     /* dest pointer       */
			while (x--) *(pd++) = *(ps++);	      /* copy the line      */
		}

		puc_y   += stride_y;
		if (y%2) {
			puc_u   += stride_uv;
			puc_v   += stride_uv;
		}
		puc_out += stride_out; 
	}
}
