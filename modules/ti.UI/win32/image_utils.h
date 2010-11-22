/**
 * @author: Mital Vora<mital.d.vora@gmail.com>
 **/

#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_

#include <kroll/kroll.h>

namespace ti
{
	namespace ImageUtils
	{
		HICON BitmapToIcon(HBITMAP bitmap, int sizeX, int sizeY);
		HBITMAP IconToBitmap(HICON icon, int sizeX, int sizeY);

		HICON LoadImageAsIcon(std::string& path, int sizeX, int sizeY);
		HBITMAP getBitMapFromPng(const std::string & path);
		HBITMAP LoadPNGAsBitmap(const std::string& path, int sizeX, int sizeY);
	}
}

#endif // _IMAGE_UTILS_H_