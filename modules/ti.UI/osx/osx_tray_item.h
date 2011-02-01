/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_OSX_TRAY_ITEM_H_
#define TI_OSX_TRAY_ITEM_H_

#include "../tray_item.h"
#include "osx_menu.h"

#include <Cocoa/Cocoa.h>

namespace ti
{
	class OSXTrayItem: public TrayItem
	{
	public:
		OSXTrayItem(const std::string& iconURL, KMethodRef cb);
		virtual ~OSXTrayItem();

		virtual void SetIcon(const std::string& iconPath);
		virtual void SetMenu(AutoMenu menu);
		virtual void SetHint(const std::string& hint);
		virtual void Remove();
		void InvokeCallback();

	private:
		NSMenu* nativeMenu;
		AutoPtr<OSXMenu> menu;
		KMethodRef callback;
		NSStatusItem* nativeItem;
	};
}

#endif

