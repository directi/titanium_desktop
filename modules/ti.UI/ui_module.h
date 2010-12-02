/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _UI_MODULE_H_
#define _UI_MODULE_H_

#include <kroll/kroll.h>
#include "ui_binding.h"

namespace ti {

	class UIModule : public kroll::Module
	{
		KROLL_MODULE_CLASS(UIModule)
		virtual void Start();

	public:
		static UIModule* GetInstance() { return instance_; }

	private:
		DISALLOW_EVIL_CONSTRUCTORS(UIModule);
		UIBinding * uiBinding;

		static UIModule* instance_;
	};
}

#endif
