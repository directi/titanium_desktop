/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_K_ACCESSOR_H_
#define _KR_K_ACCESSOR_H_

#include "../base.h"

#include "binding_declaration.h"

namespace kroll
{
	typedef std::map<std::string, KMethodRef> AccessorMap;

	class KROLL_API KAccessor
	{
	protected:
		KAccessor();

		void RecordAccessor(const std::string& name, KValueRef value);
		bool HasGetterFor(std::string name);
		KValueRef UseGetter(std::string name, KValueRef existingValue);
		bool UseSetter(std::string name, KValueRef newValue, KValueRef existingValue);

	private:
		void DoMap(std::string name, KValueRef accessor, AccessorMap& map);
		KMethodRef FindAccessor(std::string& name, AccessorMap& map);

		DISALLOW_EVIL_CONSTRUCTORS(KAccessor);
		AccessorMap getterMap;
		AccessorMap setterMap;
	};
}

#endif
