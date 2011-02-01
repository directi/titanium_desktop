/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _OSX_MENU_ITEM_H_
#define _OSX_MENU_ITEM_H_

#include <kroll/kroll.h>
#include "../menu_item.h"
#include <Cocoa/Cocoa.h>

namespace ti
{
	class OSXMenuItem : public MenuItem
	{
	public:
		OSXMenuItem(MenuItemType type);
		virtual ~OSXMenuItem();

		virtual void SetLabelImpl(const std::string &newLabel);
		virtual void SetIconImpl(const std::string &newIconPath);
		virtual void SetStateImpl(bool newState);
		virtual void SetSubmenuImpl(AutoMenu newSubmenu);
		virtual void SetEnabledImpl(bool enabled);

		NSMenuItem* CreateNative(bool registerNative=true);
		void DestroyNative(NSMenuItem* realization);
		void UpdateNativeMenuItems();
		virtual void HandleClickEvent(KObjectRef source);

	private:
		static void SetNSMenuItemTitle(NSMenuItem* item, const std::string& title);
		static void SetNSMenuItemState(NSMenuItem* item, bool state);
		static void SetNSMenuItemIconPath(
			NSMenuItem* item, std::string& iconPath, NSImage* image = nil);
		static void SetNSMenuItemSubmenu(
			NSMenuItem* item, AutoMenu submenu, bool registerNative=true);
		static void SetNSMenuItemEnabled(NSMenuItem* item, bool enabled);

		std::vector<NSMenuItem*> nativeItems;
	};
}
#endif
