/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_WIN32_TRAY_ITEM_H_
#define TI_WIN32_TRAY_ITEM_H_

#include <kroll/kroll.h>
#include <windows.h>
#include <shellapi.h>

#include "../tray_item.h"

namespace ti
{
	class Win32TrayItem: public TrayItem
	{
	public:
		Win32TrayItem(const std::string& iconURL, KValueRef cbSingleClick);
		virtual ~Win32TrayItem();
		virtual void SetIcon(const std::string& iconPath);
		virtual void SetMenu(AutoMenu menu);
		virtual void SetHint(const std::string& hint);
		virtual void ShowBalloonMessage(const std::string & title, const std::string & message);
		virtual void ResetBalloonMessage(const std::string & title);

		void Remove();
		void ShowTrayMenu();
		void HandleRightClick();
		void HandleLeftClick();
		void HandleDoubleLeftClick();
		UINT GetId() const;

		static bool MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK DoubleClickTimerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		HMENU oldNativeMenu;
		NOTIFYICONDATA* trayIconData;
		bool is_double_clicked;

		static std::vector<Win32TrayItem *> trayItems;
		static UINT trayClickedMessage;
		static UINT trayCreatedMessage;
	};
}
#endif /* TI_WIN32_TRAY_ITEM_H_ */
