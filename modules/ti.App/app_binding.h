/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _APP_BINDING_H_
#define _APP_BINDING_H_

#include <kroll/kroll.h>

namespace ti
{
	class AppBinding : public KAccessorObject
	{
	public:
		AppBinding(Host *host,KObjectRef);
		virtual ~AppBinding();

	private:
		Host* host;
		KObjectRef global;

		void GetID(const ValueList& args, KValueRef result);
		void GetName(const ValueList& args, KValueRef result);
		void GetTitle(const ValueList& args, KValueRef result);
		void GetVersion(const ValueList& args, KValueRef result);
		void GetPublisher(const ValueList& args, KValueRef result);
		void GetCopyright(const ValueList& args, KValueRef result);
		void GetDescription(const ValueList& args, KValueRef result);
		void GetURL(const ValueList& args, KValueRef result);
		void GetGUID(const ValueList& args, KValueRef result);
		void GetIcon(const ValueList& args, KValueRef result);
		void GetPath(const ValueList& args, KValueRef result);
		void GetHome(const ValueList& args, KValueRef result);
		void GetArguments(const ValueList& args, KValueRef result);
		void AppURLToPath(const ValueList& args, KValueRef result);
		void SetMenu(const ValueList& args, KValueRef result);
		void Exit(const ValueList& args, KValueRef result);
		void Exec(const ValueList& args, KValueRef result);
		void Restart(const ValueList& args, KValueRef result);
		void StdOut(const ValueList& args, KValueRef result);
		void StdErr(const ValueList& args, KValueRef result);
		void StdIn(const ValueList& args, KValueRef result);

	protected:
		void Setup();
	};
}

#endif
