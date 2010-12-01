/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "ui_module.h"

namespace ti
{
	TrayItem::TrayItem(const std::string& iconURL) :
		KEventObject("UI.TrayItem"),
		iconURL(iconURL),
		iconPath(URLUtils::URLToPath(iconURL)),
		removed(false)
	{
		this->SetMethod("setIcon", &TrayItem::_SetIcon);
		this->SetMethod("getIcon", &TrayItem::_GetIcon);
		this->SetMethod("setMenu", &TrayItem::_SetMenu);
		this->SetMethod("getMenu", &TrayItem::_GetMenu);
		this->SetMethod("setHint", &TrayItem::_SetHint);
		this->SetMethod("getHint", &TrayItem::_GetHint);

#ifdef WIN32
		/**
		 * @tiapi(method=True,name=UI.Tray.showBalloonMessage,since=0.8)
		 * @tiapi Shows the balloon popup with given title and message
		 */
		this->SetMethod("showBalloonMessage", &TrayItem::_ShowBalloonMessage);
		/**
		 * @tiapi(method=True,name=UI.Tray.resetBalloonMessage,since=0.8)
		 * @tiapi reset the balloon popup with given title
		 */
		this->SetMethod("resetBalloonMessage", &TrayItem::_ResetBalloonMessage);
#endif

		/**
		 * @tiapi(method=True,name=UI.Tray.remove,since=0.2) Removes a TrayItem
		 */
		this->SetMethod("remove", &TrayItem::_Remove);
	}

	TrayItem::~TrayItem()
	{
	}

	void TrayItem::_SetIcon(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setIcon", "s");

		this->iconPath = this->iconURL = "";
		if (args.size() > 0) {
			this->iconURL = args.GetString(0);
			this->iconPath = URLUtils::URLToPath(iconURL);
		}

		if (!removed)
			this->SetIcon(this->iconPath);
	}

	void TrayItem::_GetIcon(const ValueList& args, KValueRef result)
	{
		result->SetString(this->iconURL);
	}

	void TrayItem::_SetMenu(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setMenu", "o|0");
		AutoMenu menu(0); // A NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsObject())
		{
			menu = args.at(0)->ToObject().cast<Menu>();
		}

		if (!removed)
			this->SetMenu(menu);

		this->menu = menu;
	}

	void TrayItem::_GetMenu(const ValueList& args, KValueRef result)
	{
		result->SetObject(this->menu);
	}

	void TrayItem::_SetHint(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setHint", "s|0");
		this->hint = "";
		if (args.size() > 0) {
			hint = args.GetString(0);
		}

		if (!removed)
			this->SetHint(hint);
	}

	void TrayItem::_GetHint(const ValueList& args, KValueRef result)
	{
		result->SetString(this->hint);
	}

	void TrayItem::_Remove(const ValueList& args, KValueRef result)
	{
		if (removed)
			return;

		this->Remove();
		UIBinding::GetInstance()->UnregisterTrayItem(this);
		removed = true;
	}
#ifdef WIN32
	void TrayItem::_ShowBalloonMessage(const ValueList& args, KValueRef result)
	{
		std::string title = args.GetString(0);
		std::string msg = args.GetString(1);
		this->ShowBalloonMessage(title, msg);
	}

	void TrayItem::_ResetBalloonMessage(const ValueList& args, KValueRef result)
	{
		std::string title = args.GetString(0);
		this->ResetBalloonMessage(title);
	}
#endif

}
