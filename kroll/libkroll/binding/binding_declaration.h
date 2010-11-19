
#ifndef _BINDING_DECLARATION_H_
#define _BINDING_DECLARATION_H_

#ifdef NO_AUTOPTR
#include "UnAutoPtr.h"
#else
#include <Poco/AutoPtr.h>
using Poco::AutoPtr;
#endif

namespace kroll
{
	class Value;
	class KObject;
	class KMethod;
	class KList;
	class Bytes;
	class ArgList;
	class StaticBoundObject;
	class StaticBoundMethod;

#ifdef NO_AUTOPTR
	typedef UnAutoPtr<Value> KValueRef;
	typedef UnAutoPtr<KObject> KObjectRef;
	typedef UnAutoPtr<KMethod> KMethodRef;
	typedef UnAutoPtr<KList> KListRef;
	typedef UnAutoPtr<Bytes> BytesRef;
#else
	typedef AutoPtr<Value> KValueRef;
	typedef AutoPtr<KObject> KObjectRef;
	typedef AutoPtr<KMethod> KMethodRef;
	typedef AutoPtr<KList> KListRef;
	typedef AutoPtr<Bytes> BytesRef;
#endif
	typedef ArgList ValueList;
};

#endif // _BINDING_DECLARATION_H_