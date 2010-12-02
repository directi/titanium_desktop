/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TRAY_ITEM_H_
#define _TRAY_ITEM_H_

#include <kroll/kroll.h>
#include "menu_item.h"

namespace ti
{
	class TrayItem : public KEventObject
	{

	public:
		TrayItem(const std::string& iconURL);
		virtual ~TrayItem();

		virtual void SetIcon(const std::string& iconPath) = 0;
		virtual void SetMenu(AutoMenu menu) = 0;
		virtual void SetHint(const std::string& hint) = 0;
#ifdef WIN32
		virtual void ShowBalloonMessage(const std::string & title, const std::string & message) = 0;
		void _ShowBalloonMessage(const ValueList& args, KValueRef result);
		virtual void ResetBalloonMessage(const std::string & title) = 0;
		void _ResetBalloonMessage(const ValueList& args, KValueRef result);
#endif
		virtual void Remove() = 0;

		void _SetIcon(const ValueList& args, KValueRef result);
		void _SetMenu(const ValueList& args, KValueRef result);
		void _SetHint(const ValueList& args, KValueRef result);
		void _GetIcon(const ValueList& args, KValueRef result);
		void _GetMenu(const ValueList& args, KValueRef result);
		void _GetHint(const ValueList& args, KValueRef result);
		void _Remove(const ValueList& args, KValueRef result);

	protected:
		AutoMenu menu;
		std::string iconURL;
		std::string iconPath;
		std::string hint;
		bool removed;
	};
}

#endif
