/**
 * @author: Mital Vora<mital.d.vora@gmail.com>
 **/

#include "image_utils.h"
#include <libpng13/png.h>

namespace ti
{
	HICON ImageUtils::BitmapToIcon(HBITMAP bitmap, int sizeX, int sizeY)
	{
		if (!bitmap)
			return 0;

		HDC hdc = GetDC(NULL);
		HDC memdc = ::CreateCompatibleDC(hdc);
		HBITMAP mask = ::CreateCompatibleBitmap( memdc, sizeX, sizeY );

		ICONINFO ii;
		ii.fIcon = TRUE;
		ii.hbmMask = mask;
		ii.hbmColor = bitmap;
		HICON icon = ::CreateIconIndirect( &ii );

		::DeleteDC( memdc );
		::DeleteObject( mask );

		return icon;
	}

	HBITMAP ImageUtils::IconToBitmap(HICON icon, int sizeX, int sizeY)
	{
		if (!icon)
			return 0;

		HDC hdc = GetDC(NULL);
		HDC memdc = CreateCompatibleDC(hdc);
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, sizeX, sizeY);
		HBITMAP holdbitmap = (HBITMAP) SelectObject(memdc, bitmap);

		RECT rect = { 0, 0, sizeX, sizeY };
		SetBkColor(memdc, RGB(255, 255, 255));
		ExtTextOut(memdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		DrawIconEx(memdc, 0, 0, icon, sizeX, sizeY, 0, NULL, DI_NORMAL);

		SelectObject(hdc, holdbitmap);
		DeleteDC(memdc);

		return bitmap;
	}


	// prototypes, optionally connect these to whatever you log errors with 
	void PNGAPI user_error_fn(png_structp png, png_const_charp sz) { }
	void PNGAPI user_warning_fn(png_structp png, png_const_charp sz) { }

	HBITMAP ImageUtils::getBitMapFromPng(const std::string & path)
	{
		HBITMAP hbm = NULL;
		bool retVal = false;
		int size = 0;
		// check the header first
		FILE *fp = fopen(path.c_str(), "rb");
		if (!fp)
			return false;
		BYTE header[8];
		fread(header, 1, 8, fp);
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fclose(fp);
		if (png_sig_cmp(header, 0, 8))
			return false;
		// now allocate stuff
		png_structp png_ptr =
			png_create_read_struct(PNG_LIBPNG_VER_STRING,
			NULL, user_error_fn, user_warning_fn);
		if (!png_ptr)
			return false;
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			png_destroy_read_struct(&png_ptr,
				(png_infopp)NULL, (png_infopp)NULL);
			return false;
		}

		png_infop end_info = png_create_info_struct(png_ptr);
		if (!end_info)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr,
				(png_infopp)NULL);
			return false;
		}

		fp = fopen(path.c_str(), "rb");
		if (fp)
		{
			png_init_io(png_ptr, fp);

			// should really use png_set_rows() to allocate space first, rather than doubling up

			png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING, NULL);

			fclose(fp);

			png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);//new png_bytep[info_ptr->height];

			// now for a tonne of ugly DIB setup crap

			int width = info_ptr->width;
			int height = info_ptr->height;
			int bpp = info_ptr->channels * 8;
			int memWidth = (width * (bpp >> 3) + 3) & ~3;

			LPBITMAPINFO lpbi = (LPBITMAPINFO) new char[sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))];

			// create a greyscale palette
			for (int a_i = 0; a_i < 256; a_i++)
			{
				lpbi->bmiColors[a_i].rgbRed = (BYTE)a_i;
				lpbi->bmiColors[a_i].rgbGreen = (BYTE)a_i;
				lpbi->bmiColors[a_i].rgbBlue = (BYTE)a_i;
				lpbi->bmiColors[a_i].rgbReserved = 0;
			}

			lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			lpbi->bmiHeader.biWidth = width;
			lpbi->bmiHeader.biHeight = -height; // must be negative for top down
			lpbi->bmiHeader.biPlanes = 1;
			lpbi->bmiHeader.biBitCount = bpp;
			lpbi->bmiHeader.biCompression = BI_RGB;
			lpbi->bmiHeader.biSizeImage = memWidth * height;
			lpbi->bmiHeader.biXPelsPerMeter = 0;
			lpbi->bmiHeader.biYPelsPerMeter = 0;
			lpbi->bmiHeader.biClrUsed = 0;
			lpbi->bmiHeader.biClrImportant = 0;

			BYTE * pixelData;
			hbm = CreateDIBSection(NULL, lpbi, DIB_RGB_COLORS, (void **)&pixelData, NULL, 0 );
			if (hbm && pixelData)
			{
				// now copy the rows
				for (int i = 0; i < height; i++)
					memcpy(pixelData + memWidth * i, row_pointers[i], width * info_ptr->channels);
			}

			delete (char*) lpbi;
		}

		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return hbm;
	}

	HBITMAP ImageUtils::LoadPNGAsBitmap(const std::string& path, int sizeX, int sizeY)
	{
		HBITMAP hbm = getBitMapFromPng(path);
		HBITMAP hbmScaled = (HBITMAP)CopyImage(hbm,IMAGE_BITMAP,sizeX,-sizeY,LR_COPYRETURNORG);
		return hbmScaled;
	}

} // end ti