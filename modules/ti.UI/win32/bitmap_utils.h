#ifndef _BITMAP_UTILS_H_
#define _BITMAP_UTILS_H_

#include <kroll/kroll.h>

namespace ti
{
	namespace BitmapUtils
	{
		HBITMAP ScaleBitmap(HBITMAP hBmp, WORD wNewWidth, WORD wNewHeight);

		BITMAPINFO *PrepareRGBBitmapInfo(WORD wWidth, WORD wHeight);

		float *CreateCoeff(int nLen, int nNewLen, BOOL bShrink);

		void ShrinkData(BYTE *pInBuff, 
			WORD wWidth, 
			WORD wHeight,
			BYTE *pOutBuff, 
			WORD wNewWidth, 
			WORD wNewHeight);

		void EnlargeData(BYTE *pInBuff, 
			WORD wWidth, 
			WORD wHeight,
			BYTE *pOutBuff, 
			WORD wNewWidth, 
			WORD wNewHeight);
	}
}

#endif // _BITMAP_UTILS_H_