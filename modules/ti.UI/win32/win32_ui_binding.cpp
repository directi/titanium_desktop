/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"
#include <libpng13/png.h>
#define _WINSOCKAPI_
#include <cstdlib>
#include <sstream>
#include <windows.h>

#include <webkit/QuickCurlSettingsExport.h>

using std::vector;
namespace ti
{
	UINT Win32UIBinding::nextItemId = NEXT_ITEM_ID_BEGIN;
	vector<HICON> Win32UIBinding::loadedICOs;
	vector<HBITMAP> Win32UIBinding::loadedBMPs;

	Win32UIBinding::Win32UIBinding(Module *uiModule, Host *host) :
		UIBinding(host),
		menu(0),
		contextMenu(0),
		iconPath("")
	{
		// Initialize common controls so that our Win32 native
		// components look swanky.
		INITCOMMONCONTROLSEX InitCtrlEx;
		InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCtrlEx.dwICC = 0x00004000; //ICC_STANDARD_CLASSES;
		InitCommonControlsEx(&InitCtrlEx);
		
		// Set the cert path for Curl so that HTTPS works properly.
		// We are using _puetenv here since WebKit uses getenv internally 
		// which is incompatible with the Win32 envvar API.
		std::wstring pemPath = ::UTF8ToWide(FileUtils::Join(
			Host::GetInstance()->GetApplication()->getRuntimePath().c_str(),
			"rootcert.pem", 0));
		std::wstring var = L"CURL_CA_BUNDLE_PATH=" + pemPath;
		_wputenv(var.c_str());

		setProxyCallback(ProxyForURLCallback);
		// Hook app:// and ti:// URL support to WebKit
		setTitaniumProtocolResolver(TitaniumProtocolResolver);

		std::string cookieJarFilename(FileUtils::Join(
			Host::GetInstance()->GetApplication()->GetDataPath().c_str(),
			"cookies.dat", 0));
		setCookieJarFileName(cookieJarFilename.c_str());
	}
	
	Win32UIBinding::~Win32UIBinding()
	{
	}

	AutoMenu Win32UIBinding::CreateMenu()
	{
		return new Win32Menu();
	}

	AutoMenuItem Win32UIBinding::CreateMenuItem()
	{
		return new Win32MenuItem(MenuItem::NORMAL);
	}

	AutoMenuItem Win32UIBinding::CreateSeparatorMenuItem()
	{
		return new Win32MenuItem(MenuItem::SEPARATOR);
	}

	AutoMenuItem Win32UIBinding::CreateCheckMenuItem()
	{
		return new Win32MenuItem(MenuItem::CHECK);
	}

	void Win32UIBinding::SetMenu(AutoMenu newMenu)
	{
		this->menu = newMenu.cast<Win32Menu>();
	}

	void Win32UIBinding::SetContextMenu(AutoMenu newMenu)
	{
		this->contextMenu = newMenu.cast<Win32Menu>();
	}

	void Win32UIBinding::SetIcon(std::string& iconPath)
	{
		if (!FileUtils::IsFile(iconPath))
		{
			this->iconPath = "";
		}
		else
		{
			this->iconPath = iconPath;
		}
	}

	AutoPtr<TrayItem> Win32UIBinding::AddTray(std::string& iconPath, KMethodRef cbSingleClick)
	{
		AutoPtr<TrayItem> trayItem = new Win32TrayItem(iconPath, cbSingleClick);
		return trayItem;
	}

	long Win32UIBinding::GetIdleTime()
	{
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		return (int)idleTicks;
	}

	AutoMenu Win32UIBinding::GetMenu()
	{
		return this->menu;
	}

	AutoMenu Win32UIBinding::GetContextMenu()
	{
		return this->contextMenu;
	}

	std::string& Win32UIBinding::GetIcon()
	{
		return this->iconPath;
	}

	/*static*/
	HBITMAP Win32UIBinding::LoadImageAsBitmap(std::string& path, int sizeX, int sizeY)
	{
		UINT flags = LR_DEFAULTSIZE | LR_LOADFROMFILE |
			LR_LOADTRANSPARENT | LR_CREATEDIBSECTION;

		std::wstring widePath(::UTF8ToWide(path));
		const char* ext = path.c_str() + path.size() - 4;
		HBITMAP h = 0;
		if (_stricmp(ext, ".ico") == 0)
		{
			HICON hicon = (HICON) LoadImageW(NULL, widePath.c_str(), IMAGE_ICON,
				sizeX, sizeY, LR_LOADFROMFILE);
			h = Win32UIBinding::IconToBitmap(hicon, sizeX, sizeY);
			DestroyIcon(hicon);
		}
		else if (_stricmp(ext, ".bmp") == 0)
		{
			h = (HBITMAP) LoadImageW(
				NULL, widePath.c_str(), IMAGE_BITMAP, sizeX, sizeY, flags);
		}
		else if (_stricmp(ext, ".png") == 0)
		{
			h = LoadPNGAsBitmap(path, sizeX, sizeY);
		}
		else
		{
			throw ValueException::FromFormat("Unsupported image file: %s", path);
		}

		loadedBMPs.push_back(h);
		return h;
	}

	/*static*/
	HICON Win32UIBinding::LoadImageAsIcon(std::string& path, int sizeX, int sizeY)
	{
		UINT flags = LR_DEFAULTSIZE | LR_LOADFROMFILE |
			LR_LOADTRANSPARENT | LR_CREATEDIBSECTION;

		const char* ext = path.c_str() + path.size() - 4;
		std::wstring widePath(::UTF8ToWide(path));
		HICON h = 0;
		if (_stricmp(ext, ".ico") == 0)
		{
			h = (HICON) LoadImageW(0, widePath.c_str(),
				IMAGE_ICON, sizeX, sizeY, LR_LOADFROMFILE);
		}
		else if (_stricmp(ext, ".bmp") == 0)
		{
			HBITMAP bitmap = (HBITMAP) LoadImageW(0, widePath.c_str(),
				IMAGE_BITMAP, sizeX, sizeY, flags);
			h = Win32UIBinding::BitmapToIcon(bitmap, sizeX, sizeY);
			DeleteObject(bitmap);
		}
		else if (_stricmp(ext, ".png") == 0)
		{
			HBITMAP bitmap = LoadPNGAsBitmap(path, sizeX, sizeY);
			h = Win32UIBinding::BitmapToIcon(bitmap, sizeX, sizeY);
			DeleteObject(bitmap);
		}
		else
		{
			throw ValueException::FromFormat("Unsupported image file: %s", path);
		}

		loadedICOs.push_back(h);
		return (HICON) h;
	}
	
	/*static*/
	HICON Win32UIBinding::BitmapToIcon(HBITMAP bitmap, int sizeX, int sizeY)
	{
		if (!bitmap)
			return 0;

		HBITMAP bitmapMask = CreateCompatibleBitmap(GetDC(0), sizeX, sizeY);
		ICONINFO iconInfo = {0};
		iconInfo.fIcon = TRUE;
		iconInfo.hbmMask = bitmapMask;
		iconInfo.hbmColor = bitmap;
		HICON icon = CreateIconIndirect(&iconInfo);
		DeleteObject(bitmapMask);
		
		return icon;
	}

	/*static*/
	HBITMAP Win32UIBinding::IconToBitmap(HICON icon, int sizeX, int sizeY)
	{
		if (!icon)
			return 0;

		HDC hdc = GetDC(NULL);
		HDC hdcmem = CreateCompatibleDC(hdc);
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, sizeX, sizeY);
		HBITMAP holdbitmap = (HBITMAP) SelectObject(hdcmem, bitmap);

		RECT rect = { 0, 0, sizeX, sizeY };
		SetBkColor(hdcmem, RGB(255, 255, 255));
		ExtTextOut(hdcmem, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		DrawIconEx(hdcmem, 0, 0, icon, sizeX, sizeY, 0, NULL, DI_NORMAL);

		SelectObject(hdc, holdbitmap);
		DeleteDC(hdcmem);

		return bitmap;
	}

	// prototypes, optionally connect these to whatever you log errors with
 
	void PNGAPI user_error_fn(png_structp png, png_const_charp sz) { }
	void PNGAPI user_warning_fn(png_structp png, png_const_charp sz) { }


	/*static*/
	HBITMAP Win32UIBinding::LoadPNGAsBitmap(std::string& path, int sizeX, int sizeY)
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

		HBITMAP hbmScaled = BitmapUtils::ScaleBitmap(hbm, sizeX, -sizeY);
		::DeleteObject(hbm);

		return hbmScaled;
	}


/*
	cairo_surface_t* Win32UIBinding::ScaleCairoSurface(
		cairo_surface_t* oldSurface, int newWidth, int newHeight)
	{
		cairo_matrix_t scaleMatrix;
		cairo_matrix_init_scale(&scaleMatrix,
			(double) cairo_image_surface_get_width(oldSurface) / (double) newWidth,
			(double) cairo_image_surface_get_height(oldSurface) / (double) newHeight);

		cairo_pattern_t* surfacePattern = cairo_pattern_create_for_surface(oldSurface);
		cairo_pattern_set_matrix(surfacePattern, &scaleMatrix);
		cairo_pattern_set_filter(surfacePattern, CAIRO_FILTER_BEST);

		cairo_surface_t* newSurface = cairo_surface_create_similar(
			oldSurface, CAIRO_CONTENT_COLOR_ALPHA, newWidth, newHeight);
		cairo_t* cr = cairo_create(newSurface);
		cairo_set_source(cr, surfacePattern);

		/* To avoid getting the edge pixels blended with 0 alpha, which would 
		 * occur with the default EXTEND_NONE. Use EXTEND_PAD for 1.2 or newer (2) */
/*		cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REFLECT);

		 /* Replace the destination with the source instead of overlaying */
/*		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

		/* Do the actual drawing */
/*		cairo_paint(cr);
		cairo_destroy(cr);

		return newSurface;
	 }
*/

	/*static*/
	void Win32UIBinding::ReleaseImage(HANDLE handle)
	{
		vector<HICON>::iterator i = loadedICOs.begin();
		while (i != loadedICOs.end()) {
			if (*i == handle) {
				DestroyIcon(*i);
				return;
			} else {
				i++;
			}
		}

		vector<HBITMAP>::iterator i2 = loadedBMPs.begin();
		while (i2 != loadedBMPs.end()) {
			if (*i2 == handle) {
				::DeleteObject(*i2);
				return;
			} else {
				i2++;
			}
		}

	}

	/*static*/
	void Win32UIBinding::SetProxyForURL(std::string& url)
	{
		SharedPtr<Proxy> proxy(ProxyConfig::GetProxyForURL(url));
		if (!proxy.isNull())
		{
			std::stringstream proxyEnv;
			if (proxy->type == HTTP)
				proxyEnv << "http_proxy=http://";
			else if (proxy->type = HTTPS)
				proxyEnv << "HTTPS_PROXY=https://";

			if (!proxy->username.empty() || !proxy->password.empty())
				proxyEnv << proxy->username << ":" << proxy->password << "@";

			proxyEnv << proxy->host;

			if (proxy->port != 0)
				proxyEnv << ":" << proxy->port;

			std::wstring proxyEnvStr(::UTF8ToWide(proxyEnv.str()));
			_wputenv(proxyEnvStr.c_str());
		}
	}

	/*static*/
	void Win32UIBinding::ErrorDialog(std::string msg)
	{
		std::wstring msgW = ::UTF8ToWide(msg);
		MessageBox(NULL, msgW.c_str(), L"Application Error", 
			MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		UIBinding::ErrorDialog(msg);
	}

}
