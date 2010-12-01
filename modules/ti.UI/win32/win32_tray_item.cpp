/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"

namespace ti
{
	std::vector<Win32TrayItem *> Win32TrayItem::trayItems;
	UINT Win32TrayItem::trayClickedMessage =
		::RegisterWindowMessageA(PRODUCT_NAME"TrayClicked");
	UINT Win32TrayItem::trayCreatedMessage =
		::RegisterWindowMessageA("TaskbarCreated");

	Win32TrayItem::Win32TrayItem(std::string& iconURL, KValueRef cbSingleClick) :
		TrayItem(iconURL),
		oldNativeMenu(0),
		trayIconData(0)
	{
		if(cbSingleClick)
		{
			this->AddEventListener(Event::CLICKED, cbSingleClick);
		}

		HWND hwnd = Host::GetInstance()->AddMessageHandler(
			&Win32TrayItem::MessageHandler);

		NOTIFYICONDATA* notifyIconData = new NOTIFYICONDATA;
		notifyIconData->cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData->hWnd = hwnd;
		notifyIconData->uID = ++Win32UIBinding::nextItemId;
		notifyIconData->uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		notifyIconData->uCallbackMessage = trayClickedMessage;

		HICON icon = Win32UIBinding::LoadImageAsIcon(iconPath,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON));
		notifyIconData->hIcon = icon;
		notifyIconData->dwInfoFlags = NIIF_USER;

		lstrcpy(notifyIconData->szTip, L"Titanium Application");
		Shell_NotifyIcon(NIM_ADD, notifyIconData);
		this->trayIconData = notifyIconData;

		trayItems.push_back(this);
	}
	
	Win32TrayItem::~Win32TrayItem()
	{
		this->Remove();
		for(std::vector<Win32TrayItem *>::iterator
			oIter = trayItems.begin();
			oIter != trayItems.end();
		oIter++)
		{
			if(*oIter == this)
			{
				trayItems.erase(oIter);
				break;
			}
		}
	}
	
	void Win32TrayItem::SetIcon(std::string& iconPath)
	{
		if (!this->trayIconData)
			return;

		HICON icon = Win32UIBinding::LoadImageAsIcon(iconPath,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON));
		this->trayIconData->hIcon = icon;
		Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);
	}
	
	void Win32TrayItem::SetMenu(AutoMenu menu)
	{
		this->menu = menu;
	}
	
	void Win32TrayItem::SetHint(std::string& hint)
	{
		if (this->trayIconData)
		{
			// NotifyIconData.szTip has 128 character limit.
			ZeroMemory(this->trayIconData->szTip, 128);

			// make sure we don't overflow the static buffer.
			std::wstring hintW = ::UTF8ToWide(hint);
			lstrcpyn(this->trayIconData->szTip, hintW.c_str(), 128);

			Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);
		}
	}

	void Win32TrayItem::ShowBalloonMessage(std::string & title, std::string & message)
	{				
		// Set the flags for showing balloon, espcially NIF_INFO
		this->trayIconData->uFlags |= NIF_INFO;

		// Set the balloon title
		std::wstring wtitle(title.begin(), title.end());
		::lstrcpy(this->trayIconData->szInfoTitle, wtitle.c_str());

		// Set balloon message
		std::wstring msg(message.begin(), message.end());
		::lstrcpy(this->trayIconData->szInfo, msg.c_str());

		// Show balloon....
		::Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);

	}

	void Win32TrayItem::ResetBalloonMessage(std::string & title)
	{				
		// Set the flags for showing balloon, espcially NIF_TIP
		this->trayIconData->uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

		// Set the balloon title
		std::wstring wtitle(title.begin(), title.end());
		::lstrcpy(this->trayIconData->szTip, wtitle.c_str());

		// Show balloon....
		::Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);
	}

	
	void Win32TrayItem::Remove()
	{
		if (this->trayIconData)
		{
			Shell_NotifyIcon(NIM_DELETE, this->trayIconData);
			this->trayIconData = 0;
		}
	}

	void Win32TrayItem::HandleRightClick()
	{
		if (this->oldNativeMenu)
		{
			DestroyMenu(this->oldNativeMenu);
			this->oldNativeMenu = 0;
		}

		if (this->menu.isNull())
			return;

		AutoPtr<Win32Menu> win32menu = this->menu.cast<Win32Menu>();
		if (win32menu.isNull())
			return;

		this->oldNativeMenu = win32menu->CreateNative(false);
		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(this->trayIconData->hWnd); 
		TrackPopupMenu(this->oldNativeMenu, TPM_BOTTOMALIGN, 
			pt.x, pt.y, 0, this->trayIconData->hWnd, NULL);
		PostMessage(this->trayIconData->hWnd, WM_NULL, 0, 0);
	}

	void Win32TrayItem::HandleLeftClick()
	{
		try
		{
			this->FireEvent(Event::CLICKED);
		}
		catch (ValueException& e)
		{
			Logger* logger = Logger::Get("UI.Win32TrayItem");
			SharedString ss = e.DisplayString();
			logger->Error("Tray icon single click callback failed: %s", ss->c_str());
		}
	}
	
	void Win32TrayItem::HandleDoubleLeftClick()
	{
		try
		{
			this->FireEvent(Event::DOUBLE_CLICKED);
		}
		catch (ValueException& e)
		{
			Logger* logger = Logger::Get("UI.Win32TrayItem");
			SharedString ss = e.DisplayString();
			logger->Error("Tray icon double left click callback failed: %s", ss->c_str());
		}
	}
	
	UINT Win32TrayItem::GetId()
	{
		return this->trayIconData->uID;
	}

	/*static*/
	bool Win32TrayItem::MessageHandler(
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == trayClickedMessage)
		{
			UINT button = (UINT) lParam;
			int id = LOWORD(wParam);
			bool handled = false;

			for (size_t i = 0; i < trayItems.size(); i++)
			{
				Win32TrayItem * item = trayItems[i];

				// TODO: Disabling Double Click Support.
				// We need to revisit this logic once we are 
/*				item->is_double_clicked = false;
				if(item->GetId() == id && button == WM_LBUTTONDBLCLK)
				{
					item->is_double_clicked = true;
					KillTimer(hWnd, 100);
					item->HandleDoubleLeftClick();
					handled = true;
				}*/
				if (item->GetId() == id && button == WM_LBUTTONDOWN)
				{
					item->HandleLeftClick();
					//SetTimer(hWnd, 100, GetDoubleClickTime(), (TIMERPROC)DoubleClickTimerProc); 
					handled = true;
				}
				else if (item->GetId() == id && button == WM_RBUTTONDOWN)
				{
					item->HandleRightClick();
					handled = true;
				}
			}
			return handled;
		}
		else if (message == WM_MENUCOMMAND)
		{
			HMENU nativeMenu = (HMENU) lParam;
			UINT position = (UINT) wParam;
			return Win32MenuItem::HandleClickEvent(nativeMenu, position);
		}
		else if (message == trayCreatedMessage) 
		{
			for (size_t i = 0; i < trayItems.size(); i++)
			{
				Win32TrayItem *item = trayItems[i];
				Shell_NotifyIcon(NIM_ADD, item->trayIconData);
			}
			return false;
		}
		else
		{
			// Not handled;
			return false;
		}
	}
	
	/*static*/
	LRESULT CALLBACK Win32TrayItem::DoubleClickTimerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		bool handled = false;

		KillTimer(hWnd, 100);
		for (size_t i = 0; i < trayItems.size(); i++)
		{
			Win32TrayItem *item = trayItems[i];
			if (!(item->is_double_clicked))
			{
				item->HandleLeftClick();
			}

			item->is_double_clicked = false;
		}
		return 0;
	}
}
