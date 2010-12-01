/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WIN32_UI_BINDING_H_
#define _WIN32_UI_BINDING_H_

#define WEB_INSPECTOR_MENU_ITEM_ID 7500
#define NEXT_ITEM_ID_BEGIN 7501

namespace ti
{
	class Win32UIBinding : public UIBinding
	{
	public:
		Win32UIBinding(Module* uiModule, Host *host);
		virtual ~Win32UIBinding();

		virtual AutoMenu CreateMenu();
		virtual AutoMenuItem CreateMenuItem();
		virtual AutoMenuItem CreateCheckMenuItem();
		virtual AutoMenuItem CreateSeparatorMenuItem();

		virtual void SetMenu(AutoMenu);
		virtual void SetContextMenu(AutoMenu);
		virtual void SetIcon(std::string& iconPath);
		virtual TrayItem * AddTray(std::string& icon_path, KValueRef cbSingleClick);

		virtual AutoMenu GetMenu();
		virtual AutoMenu GetContextMenu();
		virtual long GetIdleTime();

		static HICON LoadImageAsIcon(std::string& path, int sizeX, int sizeY);
		static HBITMAP LoadImageAsBitmap(std::string& path, int sizeX, int sizeY);

		static void ReleaseImage(HANDLE);
		static void SetProxyForURL(std::string& url);
		static void ErrorDialog(std::string);

		std::string& GetIcon();
		static UINT nextItemId;

	private:
		AutoPtr<Win32Menu> menu;
		AutoPtr<Win32Menu> contextMenu;
		std::string iconPath;
		static std::vector<HICON> loadedICOs;
		static std::vector<HBITMAP> loadedBMPs;
	};
}

#endif
