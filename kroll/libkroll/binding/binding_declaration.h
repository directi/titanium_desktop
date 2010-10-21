
#ifndef _BINDING_DECLARATION_H_
#define _BINDING_DECLARATION_H_

#include <Poco/AutoPtr.h>
using Poco::AutoPtr;

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
	typedef AutoPtr<KMethod> KMethodRef;
	typedef AutoPtr<KList> KListRef;
	typedef AutoPtr<Bytes> BytesRef;
	typedef ArgList ValueList;
};

#endif // _BINDING_DECLARATION_H_