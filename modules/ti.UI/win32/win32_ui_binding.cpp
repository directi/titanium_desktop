/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "win32_ui_binding.h"
#include "win32_tray_item.h"
#include "image_utils.h"
#include "../url/url.h"

#include <shlobj.h>
#include <winbase.h>

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

	Win32UIBinding::Win32UIBinding(Host *host) :
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

	TrayItem *Win32UIBinding::AddTray(const std::string& iconPath, KValueRef cbSingleClick)
	{
		return new Win32TrayItem(iconPath, cbSingleClick);
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
	HBITMAP Win32UIBinding::LoadImageAsBitmap(const std::string& path, int sizeX, int sizeY)
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
			h = ImageUtils::IconToBitmap(hicon, sizeX, sizeY);
			DestroyIcon(hicon);
		}
		else if (_stricmp(ext, ".bmp") == 0)
		{
			h = (HBITMAP) LoadImageW(
				NULL, widePath.c_str(), IMAGE_BITMAP, sizeX, sizeY, flags);
		}
		else if (_stricmp(ext, ".png") == 0)
		{
			h = ImageUtils::LoadPNGAsBitmap(path, sizeX, sizeY);
		}
		else
		{
			throw ValueException::FromFormat("Unsupported image file: %s", path);
		}

		loadedBMPs.push_back(h);
		return h;
	}

	/*static*/
	HICON Win32UIBinding::LoadImageAsIcon(const std::string& path, int sizeX, int sizeY)
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
			h = ImageUtils::BitmapToIcon(bitmap, sizeX, sizeY);
			DeleteObject(bitmap);
		}
		else if (_stricmp(ext, ".png") == 0)
		{
			HBITMAP bitmap = ImageUtils::LoadPNGAsBitmap(path, sizeX, sizeY);
			h = ImageUtils::BitmapToIcon(bitmap, sizeX, sizeY);
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
