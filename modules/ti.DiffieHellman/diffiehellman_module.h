/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef TI_DIFFIEHELLMAN_MODULE_H_
#define TI_DIFFIEHELLMAN_MODULE_H_

#include <kroll/kroll.h>

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define TITANIUM_DIFFIEHELLMAN_API EXPORT
#elif defined(OS_WIN32)
# ifdef TITANIUM_DIFFIEHELLMAN_API_EXPORT
#  define TITANIUM_DIFFIEHELLMAN_API __declspec(dllexport)
# else
#  define TITANIUM_DIFFIEHELLMAN_API __declspec(dllimport)
# endif
# define EXPORT __declspec(dllexport)
#endif

namespace ti 
{
	class TITANIUM_DIFFIEHELLMAN_API DiffieHellmanModule : public kroll::Module,  public StaticBoundObject
	{
		KROLL_MODULE_CLASS(DiffieHellmanModule)
		
	private:
		kroll::KObjectRef binding;
                void CreateDH(const ValueList& args, KValueRef result);
	};

}
#endif
