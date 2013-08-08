#ifndef COLORSPACE_H
#define COLORSPACE_H

#pragma warning(disable:4731)

typedef void (*color_convert_func)( \
	unsigned char *py, unsigned char *pu, unsigned char *pv, \
	unsigned char *dst,\
	unsigned long pitch, \
	int width, \
	int height);

inline void yuv2yv12(
	unsigned char *py,
	unsigned char *pu,
	unsigned char *pv,
	unsigned char *dst,
	unsigned long pitch,
	int width,
	int height)
{
    register unsigned char *tmpdst;
	register unsigned char *y, *u, *v;
	
    register int i;
	register int halfh,halfw,halfp;

    y = py;
    u = pu;
    v = pv;
	
    halfw = (width>>1);
    halfh = (height>>1);
    halfp = (pitch>>1);
 
    tmpdst = dst;
    for (i=0;i<height;++i) 
	{
        memcpy(tmpdst, y, width);
		y+=width;
        tmpdst+=pitch;
    }

	tmpdst = dst + height*pitch;
	for(i=0;i<halfh;++i) 
	{
        memcpy(tmpdst,v,halfw);
		v+=halfw;
        tmpdst+=halfp;
    }

	tmpdst = dst + height*pitch*5/4;
	for (i=0;i<halfh;++i) 
	{
        memcpy(tmpdst,u,halfw);
		u+=halfw;
        tmpdst+=halfp;
    }

}

inline void yuv2uyvy16_mmx(
	unsigned char *py,		// [esp+4+16]
	unsigned char *pu,		// [esp+8+16]
	unsigned char *pv,		// [esp+12+16]
	unsigned char *dst,		// [esp+16+16]
	unsigned long pitch,	// [esp+20+16]
	int w,					// [esp+24+16]
	int h)					// [esp+28+16]
{	
	int width = w/2;

	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx
			
		mov			edx,[width];			;load width (mult of 8)
		mov			ebx,[pu]			;load source U ptr
		mov			ecx,[pv]			;load source V ptr
		mov			eax,[py]			;load source Y ptr
		mov			edi,[dst]			;load destination ptr
		mov			esi,[pitch]			;load destination pitch
		mov			ebp,[h]				;load height
			
		lea			ebx,[ebx+edx]			;bias pointers
		lea			ecx,[ecx+edx]			;(we count from -n to 0)
		lea			eax,[eax+edx*2]
		lea			edi,[edi+edx*4]
			
		neg			edx
		mov			[esp+24+16],edx
xyloop:
		movq		mm0,[ebx+edx]			;U0-U7
			
		movq		mm7,[ecx+edx]			;V0-V7
		movq		mm2,mm0					;U0-U7
			
		movq		mm4,[eax+edx*2]
		punpcklbw	mm0,mm7					;[V3][U3][V2][U2][V1][U1][V0][U0]
			
		movq		mm5,[eax+edx*2+8]
		punpckhbw	mm2,mm7					;[V7][U7][V6][U6][V5][U5][V4][U4]
			
		movq		mm1,mm0
		punpcklbw	mm0,mm4					;[Y3][V1][Y2][U1][Y1][V0][Y0][U0]
			
		punpckhbw	mm1,mm4					;[Y7][V3][Y6][U3][Y5][V2][Y4][U2]
		movq		mm3,mm2
			
		movq		[edi+edx*4+ 0],mm0
		punpcklbw	mm2,mm5					;[YB][V5][YA][U5][Y9][V4][Y8][U4]
			
		movq		[edi+edx*4+ 8],mm1
		punpckhbw	mm3,mm5					;[YF][V7][YE][U7][YD][V6][YC][U6]
			
		movq		[edi+edx*4+16],mm2
		movq		[edi+edx*4+24],mm3
			
		add			edx,8
		jnc			xyloop
			
		mov			edx,[esp+24+16]			;reload width counter
			
		test		ebp,1					;update U/V row every other row only
		jz			oddline
			
		sub			ebx,edx					;advance U pointer
		sub			ecx,edx					;advance V pointer
			
oddline:
		sub			eax,edx					;advance Y pointer
		sub			eax,edx					;advance Y pointer
			
		add			edi,esi					;advance dest ptr
			
		dec			ebp
		jne			xyloop
		
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		emms
	}
}

inline void yuv2yuyv16_mmx(
	unsigned char *py,		// [esp+4+16]
	unsigned char *pu,		// [esp+8+16]
	unsigned char *pv,		// [esp+12+16]
	unsigned char *dst,		// [esp+16+16]
	unsigned long pitch,	// [esp+20+16]
	int w,					// [esp+24+16]
	int h)					// [esp+28+16]
{
	
	int width = w/2;

	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx
			
		mov			edx,[width];			;load width (mult of 8)
		mov			ebx,[pu]			;load source U ptr
		mov			ecx,[pv]			;load source V ptr
		mov			eax,[py]			;load source Y ptr
		mov			edi,[dst]			;load destination ptr
		mov			esi,[pitch]			;load destination pitch
		mov			ebp,[h]				;load height
			
		lea			ebx,[ebx+edx]			;bias pointers
		lea			ecx,[ecx+edx]			;(we count from -n to 0)
		lea			eax,[eax+edx*2]
		lea			edi,[edi+edx*4]
			
		neg			edx
		mov			[esp+24+16],edx
xyloop:
		movq		mm0,[ebx+edx]			;U0-U7
			
		movq		mm7,[ecx+edx]			;V0-V7
		movq		mm1,mm0					;U0-U7
			
		movq		mm2,[eax+edx*2]			;Y0-Y7
		punpcklbw	mm0,mm7					;[V3][U3][V2][U2][V1][U1][V0][U0]
			
		movq		mm4,[eax+edx*2+8]		;Y8-YF
		punpckhbw	mm1,mm7					;[V7][U7][V6][U6][V5][U5][V4][U4]
			
		movq		mm3,mm2
		punpcklbw	mm2,mm0					;[V1][Y3][U1][Y2][V0][Y1][U0][Y0]
			
		movq		mm5,mm4
		punpckhbw	mm3,mm0					;[V3][Y7][U3][Y6][V2][Y5][U2][Y4]
			
		movq		[edi+edx*4+ 0],mm2
		punpcklbw	mm4,mm1					;[V5][YB][U5][YA][V4][Y9][U4][Y8]
			
		movq		[edi+edx*4+ 8],mm3
		punpckhbw	mm5,mm1					;[V7][YF][U7][YE][V6][YD][U6][YC]
			
		movq		[edi+edx*4+16],mm4
			
		movq		[edi+edx*4+24],mm5
		add			edx,8
			
		jnc			xyloop
			
		mov			edx,[esp+24+16]			;reload width counter
			
		test		ebp,1					;update U/V row every other row only
		jz			oddline
			
		sub			ebx,edx					;advance U pointer
		sub			ecx,edx					;advance V pointer
			
oddline:
		sub			eax,edx					;advance Y pointer
		sub			eax,edx					;advance Y pointer
			
		add			edi,esi					;advance dest ptr
			
		dec			ebp
		jne			xyloop
			
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		emms
	}
}

#endif // COLORSPACE_H





















