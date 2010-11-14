
#ifndef _BINDING_DECLARATION_H_
#define _BINDING_DECLARATION_H_

#include <Poco/AutoPtr.h>
using Poco::AutoPtr;
#ifdef NO_METHOD_AUTOPTR
#include "UnAutoPtr.h"
#endif

namespace kroll
{
	class Value;
	class KObject;
	class KMethod;
	class KList;
	class Bytes;
	class ArgList;

	//TODO: need to cleanup cyclic dependecies
	class StaticBoundObject;
	class StaticBoundMethod;

	typedef AutoPtr<Value> KValueRef;
	typedef AutoPtr<KObject> KObjectRef;
#ifdef NO_METHOD_AUTOPTR
	typedef UnAutoPtr<KMethod> KMethodRef;
#else
	typedef AutoPtr<KMethod> KMethodRef;
#endif
	typedef AutoPtr<KList> KListRef;
	typedef AutoPtr<Bytes> BytesRef;
	typedef ArgList ValueList;
};

#endif // _BINDING_DECLARATION_H_