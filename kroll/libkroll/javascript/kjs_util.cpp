/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "kjs_util.h"
#include "k_kjs_list.h"
#include "k_kjs_object.h"
#include "k_kjs_method.h"

#include <Poco/Mutex.h>
#include <kroll/binding/binding_declaration.h>
#include <kroll/utils/url_utils.h>
#include <kroll/utils/file_utils.h>

#define TROUBLE_SHOOT_GC 1

namespace kroll
{
namespace KJSUtil
{
	static inline Logger* GetLogger()
	{
		static Logger* logger = 0;
		if (!logger)
			logger = Logger::Get("JavaScript.KJSUtil");
		return logger;
	}

	static JSClassRef KJSKObjectClass = NULL;
	static JSClassRef KJSKMethodClass = NULL;
	static JSClassRef KJSKListClass = NULL;
	static const JSClassDefinition emptyClassDefinition =
	{
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};

	// These are all KJSK*Class class methods.
	static void GetPropertyNamesCallback(JSContextRef, JSObjectRef, JSPropertyNameAccumulatorRef);
	static bool HasPropertyCallback(JSContextRef, JSObjectRef, JSStringRef);
	static JSValueRef GetPropertyCallback(JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);
	static bool SetPropertyCallback(JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);
	static JSValueRef CallAsFunctionCallback(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	static void FinalizeCallback(JSObjectRef);
	static JSValueRef ToStringCallback(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	static JSValueRef EqualsCallback(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);

	// Private functions
	static void AddSpecialPropertyNames(KValueRef, SharedStringList, bool);
	static JSValueRef GetSpecialProperty(KValueRef, const char*, JSContextRef, KValueRef);
	static bool DoSpecialSetBehavior(KValueRef target, const char* name, KValueRef newValue);
	static JSValueRef GetFunctionPrototype(JSContextRef jsContext, JSValueRef* exception);
	static JSValueRef GetArrayPrototype(JSContextRef jsContext, JSValueRef* exception);

	class JSObjectValue 
	{
	private:
		int refCount;
		KValueRef objectValue;
		DISALLOW_EVIL_CONSTRUCTORS(JSObjectValue);
	public:
		void release() 
		{ 
			--refCount;
			if(refCount == 0) 
				delete this;
		}

		void duplicate() 
		{ 
			++refCount; 
		}

		int referenceCount()
		{
			return refCount;
		}

		JSObjectValue(KValueRef value) : objectValue(value), refCount(0) { }
		KValueRef ToValue() { return objectValue; }

		virtual ~JSObjectValue()
		{
			int valref = objectValue->referenceCount();
			int objref = objectValue->ToObject()->referenceCount();
			if(valref > 1 & objref == 1)
				objectValue->SetNull();
		}
	};

	typedef UnAutoPtr<JSObjectValue> JSObjectValueRef;

	static inline JSObjectValueRef pointerToJS(KValueRef ref)
	{
		return new JSObjectValue(ref);
	}

	static inline KValueRef pointerFromJS(JSObjectRef ref)
	{
		JSObjectValue *a = static_cast<JSObjectValue*>(JSObjectGetPrivate(ref));
		if(a) 
			return a->ToValue();
		return 0;
	}

#if TROUBLE_SHOOT_GC
	static std::map<JSObjectRef, Value*> objects;
#endif

	static inline JSObjectRef makeJSObject(JSContextRef jsContext, JSClassRef objectClass, KValueRef kValue)
	{
		JSObjectValueRef or = pointerToJS(kValue);
		or->duplicate();
		JSObjectRef r = JSObjectMake(jsContext, objectClass, or);
#if TROUBLE_SHOOT_GC
		objects[r] = kValue.get();
#endif
		return r;
	}

	static void FinalizeCallback(JSObjectRef jsObject)
	{
		JSObjectValueRef a = static_cast<JSObjectValue*>(JSObjectGetPrivate(jsObject));
#if TROUBLE_SHOOT_GC
		//std::map<JSObjectRef, Value*>::iterator i = objects.find(jsObject);
		//KObjectRef r = a->ToObject();
		objects.erase(jsObject);
#endif
		a->release();
	}



	KValueRef ToKrollValue(JSValueRef value, JSContextRef jsContext,
		JSObjectRef thisObject)
	{
		KValueRef krollValue = 0;
		JSValueRef exception = NULL;

		if (value == NULL)
		{
			GetLogger()->Error("Trying to convert NULL JSValueRef");
			return Value::Undefined;
		}

		if (JSValueIsNumber(jsContext, value))
		{
			krollValue = Value::NewDouble(JSValueToNumber(jsContext, value, &exception));
		}
		else if (JSValueIsBoolean(jsContext, value))
		{
			krollValue = Value::NewBool(JSValueToBoolean(jsContext, value));
		}
		else if (JSValueIsString(jsContext, value))
		{
			JSStringRef jsString = JSValueToStringCopy(jsContext, value, &exception);
			if (jsString)
			{
				std::string stringValue(ToChars(jsString));
				JSStringRelease(jsString);
				krollValue = Value::NewString(stringValue);
			}
		}
		else if (JSValueIsObject(jsContext, value))
		{
			JSObjectRef o = JSValueToObject(jsContext, value, &exception);
			if (o != NULL)
			{
				KValueRef value = pointerFromJS(o);
				if (value)
				{
					// This is a KJS-wrapped Kroll value: unwrap it
					return value;
				}
				else if (JSObjectIsFunction(jsContext, o))
				{
					// this is a pure JS method: proxy it
					// Don't use AutoPtrs here - let JSCore take care of references...
					KMethodRef tibm = new KKJSMethod(jsContext, o, thisObject);
					krollValue = Value::NewMethod(tibm);
				}
				else if (IsArrayLike(o, jsContext))
				{
					// this is a pure JS array: proxy it
					KListRef tibl = new KKJSList(jsContext, o);
					krollValue = Value::NewList(tibl);
				}
				else
				{
					// this is a pure JS object: proxy it
					KObjectRef tibo = new KKJSObject(jsContext, o);
					krollValue = Value::NewObject(tibo);
				}
			}
		}
		else if (JSValueIsNull(jsContext, value))
		{
			krollValue = kroll::Value::Null;
		}
		else
		{
			krollValue = kroll::Value::Undefined;
		}
		if (!krollValue.isNull() && exception == NULL)
		{
			return krollValue;
		}
		else if (exception != NULL)
		{
			throw ToKrollValue(exception, jsContext, NULL);
		}
		else
		{
			GetLogger()->Error("Failed Kroll->JS conversion with no exception!");
			throw ValueException::FromString("Conversion from Kroll value to JS value failed");
		}
	}

	JSValueRef ToJSValue(KValueRef value, JSContextRef jsContext)
	{
		JSValueRef jsValue = NULL;
		if (value->IsInt())
		{
			jsValue = JSValueMakeNumber(jsContext, value->ToInt());
		}
		else if (value->IsDouble())
		{
			jsValue = JSValueMakeNumber(jsContext, value->ToDouble());
		}
		else if (value->IsBool())
		{
			jsValue = JSValueMakeBoolean(jsContext, value->ToBool());
		}
		else if (value->IsString())
		{
			JSStringRef s = JSStringCreateWithUTF8CString(value->ToString());
			jsValue = JSValueMakeString(jsContext, s);
			JSStringRelease(s);
		}
		else if (value->IsObject())
		{
			KObjectRef obj = value->ToObject();
			AutoPtr<KKJSObject> kobj = obj.cast<KKJSObject>();
			if (!kobj.isNull() && kobj->SameContextGroup(jsContext))
			{
				// this object is actually a pure JS object
				jsValue = kobj->GetJSObject();
			}
			else
			{
				// this is a KObject that needs to be proxied
				jsValue = KObjectToJSValue(value, jsContext);
			}
		}
		else if (value->IsMethod())
		{
			KMethodRef meth = value->ToMethod();
			KKJSMethod* kmeth = dynamic_cast<KKJSMethod*>(meth.get());
			if (kmeth && kmeth->SameContextGroup(jsContext))
			{
				// this object is actually a pure JS callable object
				jsValue = kmeth->GetJSObject();
			}
			else
			{
				// this is a KMethod that needs to be proxied
				jsValue = KMethodToJSValue(value, jsContext);
			}
		}
		else if (value->IsList())
		{
			KListRef list = value->ToList();
			KKJSList* klist = dynamic_cast<KKJSList*>(list.get());
			if (klist && klist->SameContextGroup(jsContext))
			{
				// this object is actually a pure JS array
				jsValue = klist->GetJSObject();
			}
			else
			{
				// this is a KList that needs to be proxied
				jsValue = KListToJSValue(value, jsContext);
			}
		}
		else if (value->IsNull())
		{
			jsValue = JSValueMakeNull(jsContext);
		}
		else if (value->IsUndefined())
		{
			jsValue = JSValueMakeUndefined(jsContext);
		}
		else
		{
			jsValue = JSValueMakeUndefined(jsContext);
		}

		return jsValue;

	}

	JSValueRef KObjectToJSValue(KValueRef objectValue, JSContextRef jsContext)
	{
		if (KJSKObjectClass == NULL)
		{
			JSClassDefinition jsClassDefinition = emptyClassDefinition;
			jsClassDefinition.className = "Object";
			jsClassDefinition.getPropertyNames = GetPropertyNamesCallback;
			jsClassDefinition.finalize = FinalizeCallback;
			jsClassDefinition.hasProperty = HasPropertyCallback;
			jsClassDefinition.getProperty = GetPropertyCallback;
			jsClassDefinition.setProperty = SetPropertyCallback;
			KJSKObjectClass = JSClassCreate(&jsClassDefinition);
		}
		return makeJSObject(jsContext, KJSKObjectClass, objectValue);
	}

	JSValueRef KMethodToJSValue(KValueRef methodValue, JSContextRef jsContext)
	{
		if (KJSKMethodClass == NULL)
		{
			JSClassDefinition jsClassDefinition = emptyClassDefinition;
			jsClassDefinition.className = "Function";
			jsClassDefinition.getPropertyNames = GetPropertyNamesCallback;
			jsClassDefinition.finalize = FinalizeCallback;
			jsClassDefinition.hasProperty = HasPropertyCallback;
			jsClassDefinition.getProperty = GetPropertyCallback;
			jsClassDefinition.setProperty = SetPropertyCallback;
			jsClassDefinition.callAsFunction = CallAsFunctionCallback;
			KJSKMethodClass = JSClassCreate(&jsClassDefinition);
		}
		JSObjectRef jsobject = makeJSObject(jsContext, KJSKMethodClass, methodValue);
		JSValueRef functionPrototype = GetFunctionPrototype(jsContext, NULL);
		JSObjectSetPrototype(jsContext, jsobject, functionPrototype);
		return jsobject;
	}

	JSValueRef KListToJSValue(KValueRef listValue, JSContextRef jsContext)
	{

		if (KJSKListClass == NULL)
		{
			JSClassDefinition jsClassDefinition = emptyClassDefinition;
			jsClassDefinition.className = "Array";
			jsClassDefinition.getPropertyNames = GetPropertyNamesCallback;
			jsClassDefinition.finalize = FinalizeCallback;
			jsClassDefinition.hasProperty = HasPropertyCallback;
			jsClassDefinition.getProperty = GetPropertyCallback;
			jsClassDefinition.setProperty = SetPropertyCallback;
			KJSKListClass = JSClassCreate(&jsClassDefinition);
		}

		JSObjectRef jsobject = makeJSObject(jsContext, KJSKListClass, listValue);
		JSValueRef arrayPrototype = GetArrayPrototype(jsContext, NULL);
		JSObjectSetPrototype(jsContext, jsobject, arrayPrototype);
		return jsobject;
	}

	std::string ToChars(JSStringRef jsString)
	{
		size_t size = JSStringGetMaximumUTF8CStringSize(jsString);
		char* cstring = (char*) malloc(size);
		JSStringGetUTF8CString(jsString, cstring, size);
		std::string string(cstring);
		free(cstring);
		return string;
	}

	bool IsArrayLike(JSObjectRef object, JSContextRef jsContext)
	{
		bool isArrayLike = true;

		JSStringRef pop = JSStringCreateWithUTF8CString("pop");
		isArrayLike = isArrayLike && JSObjectHasProperty(jsContext, object, pop);
		JSStringRelease(pop);

		JSStringRef concat = JSStringCreateWithUTF8CString("concat");
		isArrayLike = isArrayLike && JSObjectHasProperty(jsContext, object, concat);
		JSStringRelease(concat);

		JSStringRef length = JSStringCreateWithUTF8CString("length");
		isArrayLike = isArrayLike && JSObjectHasProperty(jsContext, object, length);
		JSStringRelease(length);

		return isArrayLike;
	}

	static bool PrototypeHasFunctionNamed(JSContextRef jsContext, JSObjectRef object,
		JSStringRef name)
	{
		JSValueRef exception = NULL;

		JSValueRef prototypeValue = JSObjectGetPrototype(jsContext, object);
		JSObjectRef prototype = JSValueToObject(jsContext, prototypeValue, &exception);

		if (exception)
			return false;

		JSValueRef propValue = JSObjectGetProperty(jsContext, prototype, name, &exception);
		if (exception)
			return false;

		if (!JSValueIsObject(jsContext, propValue))
			return false;

		JSObjectRef prop = JSValueToObject(jsContext, propValue, &exception);
		return !exception && JSObjectIsFunction(jsContext, prop);
	}

	static bool HasPropertyCallback(JSContextRef jsContext, JSObjectRef jsObject,
		JSStringRef jsProperty)
	{
		KValueRef value = pointerFromJS(jsObject);
		if (value.isNull())
			return false;

		// Convert the name to a std::string.
		KObjectRef object = value->ToObject();
		std::string name(ToChars(jsProperty));

		// Special properties always take precedence. This is important
		// because even though the Array and Function prototypes have 
		// methods like toString -- we always want our special properties
		// to override those.
		SharedStringList specialProperties(new StringList());
		AddSpecialPropertyNames(value, specialProperties, true);
		for (size_t i = 0; i < specialProperties->size(); i++)
		{
			if (name == *specialProperties->at(i))
				return true;
		}

		// If the JavaScript prototype for Lists (Array) or Methods (Function) has
		// a method with the same name -- opt to use the prototype's version instead.
		// This will prevent  incompatible versions of things like pop() bleeding into
		// JavaScript.
		if ((value->IsList() || value->IsMethod()) &&
			PrototypeHasFunctionNamed(jsContext, jsObject, jsProperty))
		{
			return false;
		}

		return object->HasProperty(name.c_str());
	}

	static JSValueRef GetPropertyCallback(JSContextRef jsContext, 
		JSObjectRef jsObject, JSStringRef jsProperty, JSValueRef* jsException)
	{

		KValueRef value = pointerFromJS(jsObject);
		if (! value)
			return JSValueMakeUndefined(jsContext);

		KObjectRef object = value->ToObject();
		std::string name(ToChars(jsProperty));
		JSValueRef jsValue = NULL;
		try
		{
			KValueRef kvalue = object->Get(name.c_str());
			jsValue = GetSpecialProperty(value, name.c_str(), jsContext, kvalue);
		}
		catch (ValueException& exception)
		{
			*jsException = ToJSValue(exception.GetValue(), jsContext);
		}
		catch (std::exception &e)
		{
			KValueRef v = Value::NewString(e.what());
			*jsException = ToJSValue(v, jsContext);
		}
		catch (...)
		{
			std::string error = "Unknown exception trying to get property: ";
			error.append(name);
			KValueRef v = Value::NewString(error);
			*jsException = ToJSValue(v, jsContext);
		}

		return jsValue;
	}

	static bool SetPropertyCallback(JSContextRef jsContext, JSObjectRef jsObject,
		JSStringRef jsProperty, JSValueRef jsValue, JSValueRef* jsException)
	{
		KValueRef value = pointerFromJS(jsObject);
		if (value.isNull())
			return false;

		KObjectRef object = value->ToObject();
		bool success = false;
		std::string propertyName(ToChars(jsProperty));
		try
		{
			KValueRef newValue = ToKrollValue(jsValue, jsContext, jsObject);

			// Arrays in particular have a special behavior when
			// you do something like set the "length" property
			if (!DoSpecialSetBehavior(value, propertyName.c_str(), newValue))
			{
				object->Set(propertyName.c_str(), newValue);
			}
			success = true;
		}
		catch (ValueException& exception)
		{
			*jsException = ToJSValue(exception.GetValue(), jsContext);
		}
		catch (std::exception &e)
		{
			KValueRef v = Value::NewString(e.what());
			*jsException = ToJSValue(v, jsContext);
		}
		catch (...)
		{
			std::string error = "Unknown exception trying to set property: ";
			error.append(propertyName);
			KValueRef v = Value::NewString(error);
			*jsException = ToJSValue(v, jsContext);
		}

		return success;
	}

	static JSValueRef CallAsFunctionCallback(JSContextRef jsContext,
		JSObjectRef jsFunction, JSObjectRef jsThis, size_t argCount,
		const JSValueRef jsArgs[], JSValueRef* jsException)
	{
		KValueRef value = pointerFromJS(jsFunction);
		if (value.isNull())
			return JSValueMakeUndefined(jsContext);

		KMethodRef method = value->ToMethod();
		ValueList args;
		for (size_t i = 0; i < argCount; i++)
		{
			KValueRef argVal = ToKrollValue(jsArgs[i], jsContext, jsThis);
			args.push_back(argVal);
		}

		JSValueRef jsValue = NULL;
		try
		{
			KValueRef kvalue = method->Call(args);
			jsValue = ToJSValue(kvalue, jsContext);
		}
		catch (ValueException& exception)
		 {
			SharedString str = exception.DisplayString();
			*jsException = ToJSValue(exception.GetValue(), jsContext);
		} 
		catch (std::exception &e)
		{
			KValueRef v = Value::NewString(e.what());
			*jsException = ToJSValue(v, jsContext);
		}
		catch (...)
		{
			KValueRef v = Value::NewString("Unknown exception during Kroll method call");
			*jsException = ToJSValue(v, jsContext);
		}

		return jsValue;
	}

	static void AddSpecialPropertyNames(KValueRef value, SharedStringList props,
		bool showInvisible)
	{
		// Some attributes should be hidden unless the are requested specifically -- 
		// essentially a has_property(...) versus  get_property_list(...). An example
		// of this type of attribute is toString(). Some JavaScript code might expect
		// a "hash" object to have no methods in its property list. We don't want
		// toString() to show up in those situations.

		bool foundLength = false, foundToString = false, foundEquals = false;
		for (size_t i = 0; i < props->size(); i++)
		{
			SharedString propertyName = props->at(i);
			if (strcmp(propertyName->c_str(), "length") == 0)
				foundLength = true;
			if (strcmp(propertyName->c_str(), "toString") == 0)
				foundToString = true;
			if (strcmp(propertyName->c_str(), "equals") == 0)
				foundEquals = true;
		}

		if (!foundLength && value->IsList())
		{
			props->push_back(new std::string("length"));
		}

		if (!foundToString && showInvisible)
		{
			props->push_back(new std::string("toString"));
		}

		if (!foundEquals && showInvisible)
		{
			props->push_back(new std::string("equals"));
		}
	}

	static JSValueRef GetSpecialProperty(KValueRef value, const char* name, 
		JSContextRef jsContext, KValueRef objValue)
	{
		// Always override the length property on lists. Some languages
		// supply their own length property, which might be a method instead
		// of a number -- bad news.
		if (value->IsList() && !strcmp(name, "length"))
		{
			KListRef l = value->ToList();
			return JSValueMakeNumber(jsContext, l->Size());
		}

		// Only overload these methods if the value in our object is not a
		// method We want the user to be able to supply their own versions,
		// but we don't want JavaScript code to freak out in situations where
		// Kroll objects use attributes with the same name that aren't methods.
		if (!objValue->IsMethod())
		{
			if (!strcmp(name, "toString"))
			{
				JSStringRef s = JSStringCreateWithUTF8CString("toString");
				JSValueRef toString = JSObjectMakeFunctionWithCallback(
					jsContext, s, &ToStringCallback);
				JSStringRelease(s);
				return toString;
			}

			if (!strcmp(name, "equals"))
			{
				JSStringRef s = JSStringCreateWithUTF8CString("equals");
				JSValueRef equals = JSObjectMakeFunctionWithCallback(
					jsContext, s, &EqualsCallback);
				JSStringRelease(s);
				return equals;
			}
		}

		// Otherwise this is just a normal JS value
		return ToJSValue(objValue, jsContext);
	}

	static bool DoSpecialSetBehavior(KValueRef target, const char* name, KValueRef newValue)
	{
		// We only do something special if we are trying to set
		// the length property of a list to a new int value.
		if (strcmp(name, "length") || !target->IsList() || !newValue->IsInt())
		{
			return false;
		}
		target->ToList()->ResizeTo(newValue->ToInt());
		return true;
	}

	static JSValueRef ToStringCallback(JSContextRef jsContext,
		JSObjectRef jsFunction, JSObjectRef jsThis, size_t argCount,
		const JSValueRef args[], JSValueRef* exception)
	{
		KValueRef value = pointerFromJS(jsThis);
		if (value.isNull())
			return JSValueMakeUndefined(jsContext);

		SharedString ss = value->DisplayString(2);
		KValueRef dsv = Value::NewString(ss);
		return ToJSValue(dsv, jsContext);
	}

	static JSValueRef EqualsCallback(JSContextRef jsContext, JSObjectRef function,
		JSObjectRef jsThis, size_t numArgs, const JSValueRef args[],
		JSValueRef* exception)
	{
		KValueRef value = pointerFromJS(jsThis);
		if (value.isNull() || numArgs < 1)
		{
			return JSValueMakeBoolean(jsContext, false);
		}

		// Ensure argument is a JavaScript object
		if (!JSValueIsObject(jsContext, args[0]))
		{
			return JSValueMakeBoolean(jsContext, false);
		}

		// Ensure argument is a Kroll JavaScript
		JSObjectRef otherObject = JSValueToObject(jsContext, args[0], NULL);
		KValueRef otherValue = pointerFromJS(otherObject);
		if (otherValue.isNull())
		{
			return JSValueMakeBoolean(jsContext, false);
		}

		// Test equality
		return JSValueMakeBoolean(jsContext, value->Equals(otherValue));
	}

	static void GetPropertyNamesCallback(JSContextRef jsContext,
		JSObjectRef jsObject, JSPropertyNameAccumulatorRef jsProperties)
	{
		KValueRef value = pointerFromJS(jsObject);
		if (value.isNull())
			return;

		KObjectRef object = value->ToObject();
		SharedStringList props = object->GetPropertyNames();
		AddSpecialPropertyNames(value, props, false);
		for (size_t i = 0; i < props->size(); i++)
		{
			SharedString propertyName = props->at(i);
			JSStringRef name = JSStringCreateWithUTF8CString(propertyName->c_str());
			JSPropertyNameAccumulatorAddName(jsProperties, name);
			JSStringRelease(name);
		}
	}

	JSGlobalContextRef CreateGlobalContext()
	{
		JSGlobalContextRef jsContext = JSGlobalContextCreate(0);
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);

		JSValueRef jsAPI = ToJSValue(Value::NewObject(GlobalObject::GetInstance()), jsContext);

		JSStringRef propertyName = JSStringCreateWithUTF8CString(PRODUCT_NAME);
		JSObjectSetProperty(jsContext, globalObject, propertyName,
			jsAPI, kJSPropertyAttributeNone, NULL);
		JSStringRelease(propertyName);

		RegisterContext(globalObject, jsContext);
		return jsContext;
	}

	static std::map<JSObjectRef, JSContextRef> jsContextMap;
	static Poco::Mutex jsContextMapMutex;

	void RegisterContext(JSObjectRef globalObject, JSContextRef globalContext)
	{
		Poco::Mutex::ScopedLock lock(jsContextMapMutex);
		std::map<JSObjectRef, JSContextRef>::iterator i = jsContextMap.find(globalObject);
		if (i == jsContextMap.end())
		{
			jsContextMap[globalObject] = globalContext;
			//JSValueProtect(globalContext, globalObject);
		}
	}

	void UnregisterContext(JSObjectRef globalObject, JSContextRef globalContext)
	{
		Poco::Mutex::ScopedLock lock(jsContextMapMutex);
		std::map<JSObjectRef, JSContextRef>::iterator i = jsContextMap.find(globalObject);
		if(i != jsContextMap.end()) 
		{
			//JSValueUnprotect(globalContext, globalObject);
			jsContextMap.erase(i);
		}
#if DEBUG
		else 
			fprintf(stderr, "Yikes, context not found\n");
#endif
	}
	
	JSContextRef GetGlobalContext(JSObjectRef object, JSContextRef context)
	{
		Poco::Mutex::ScopedLock lock(jsContextMapMutex);
		if (jsContextMap.find(object) == jsContextMap.end())
		{
			GetLogger()->Error("Yikes need to return a NULL context!!");
			return 0;
		}
		else
		{
			return jsContextMap[object];
		}
	}

	typedef std::map<JSObjectRef, int> JSObjectInContextRefCounter;
	typedef std::map<JSContextRef, JSObjectInContextRefCounter*> JSObjectRefCounter;
	static JSObjectRefCounter jsObjectRefCounter;

#if TROUBLE_SHOOT_GC
	typedef std::map<KObject*, Value*> SurvivingObjects;
	static SurvivingObjects survivingObjects;
#endif

	static Poco::Mutex protectedObjectsMutex;

	static inline void _addContext(JSContextRef globalContext)
	{
		JSObjectRefCounter::iterator ourContext = jsObjectRefCounter.find(globalContext);
		if(ourContext == jsObjectRefCounter.end()) 
		{
			JSObjectRef globalObject = JSContextGetGlobalObject(globalContext);
			RegisterContext(globalObject, globalContext);
			jsObjectRefCounter[globalContext] = new JSObjectInContextRefCounter();
		}
	}

	static inline void _delContext(JSContextRef globalContext) 
	{
#ifdef NDEBUG
		KEventObject::CleanupListenersFromContext(globalContext);
#endif
		JSObjectRef globalObject = JSContextGetGlobalObject(globalContext);
		UnregisterContext(globalObject, globalContext);
		jsObjectRefCounter.erase(globalContext);
	}

	static void GarbageCollect() 
	{
		ValueList args;
		RunOnMainThread(new KFunctionPtrMethod(&JavaScriptModuleInstance::GarbageCollect), args, false);
	}

	void ProtectContext(JSContextRef globalContext)
	{
		Poco::Mutex::ScopedLock lock(protectedObjectsMutex);
		_addContext(globalContext);
	}

	void ProtectContextAndValue(JSContextRef globalContext, JSObjectRef value)
	{
		Poco::Mutex::ScopedLock lock(protectedObjectsMutex);
		ProtectContext(globalContext);
		JSObjectRefCounter::iterator ourCtx = jsObjectRefCounter.find(globalContext);
		if(ourCtx == jsObjectRefCounter.end())
		{
			GetLogger()->Error("No reference counter map found for globalContext");
			return;
		}
		JSObjectInContextRefCounter::iterator ourRef = ourCtx->second->find(value);
		if(ourRef == ourCtx->second->end())
		{
			(*ourCtx->second)[value] = 1;
			JSValueProtect(globalContext, value);
		} 
		else
		{
			ourRef->second++;
		}
	}

	void UnprotectContextAndValue(JSContextRef globalContext, JSObjectRef value)
	{
		Poco::Mutex::ScopedLock a(protectedObjectsMutex);
		JSObjectRefCounter::iterator ourCtx = jsObjectRefCounter.find(globalContext);
		if(ourCtx == jsObjectRefCounter.end())
		{
#if DEBUG
			fprintf(stderr, "Asked to unprotect a JS context with no object list!\n");
#endif
			return;
		}
		JSObjectInContextRefCounter::iterator ourRef = ourCtx->second->find(value);
		if(ourRef == ourCtx->second->end())
		{
#if DEBUG
			fprintf(stderr, "Asked to unprotect a JS value with no object count!\n");
#endif
		} 
		else
		{
			ourRef->second--;
			if(ourRef->second == 0)
				JSValueUnprotect(globalContext, value);
			if(ourRef->second <= 0)
				ourCtx->second->erase(ourRef);
		}
		UnprotectContext(globalContext);
	}

	void UnprotectContext(JSContextRef globalContext, bool force)
	{
		Poco::Mutex::ScopedLock a(protectedObjectsMutex);
		JSObjectRefCounter::iterator ourContext = jsObjectRefCounter.find(globalContext);
		if(ourContext != jsObjectRefCounter.end()) 
		{
			if(force)
			{
#if NDEBUG
				KEventObject::CleanupListenersFromContext(globalContext);
#endif
				JSObjectInContextRefCounter* objRefs = ourContext->second;
				if(objRefs)
				{
					JSObjectInContextRefCounter::iterator i = objRefs->begin();
					while(i != objRefs->end())
					{						
						JSValueUnprotect(globalContext, i->first);
#if TROUBLE_SHOOT_GC
						KValueRef t = pointerFromJS(i->first);
						if(! t.isNull())
							survivingObjects[t->ToObject().get()] = t.get();
#endif
						i++;
					}
					objRefs->clear();
					GarbageCollect();
				}
			}
			if(ourContext->second->size() == 0)
			{
				_delContext(globalContext);
			}
		}
#if DEBUG
		else 
			fprintf(stderr, "Asked to unprotect an unknown js context\n");
#endif
	}

	KValueRef Evaluate(JSContextRef jsContext, const char* script, const char* url)
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef scriptContents = JSStringCreateWithUTF8CString(script);
		JSStringRef jsURL = JSStringCreateWithUTF8CString(url);
		JSValueRef exception = NULL;

		JSValueRef returnValue = JSEvaluateScript(jsContext, scriptContents, globalObject, 
			jsURL, 0, &exception);

		JSStringRelease(jsURL);
		JSStringRelease(scriptContents);

		if (exception)
			throw ValueException(ToKrollValue(exception, jsContext, NULL));

		return ToKrollValue(returnValue, jsContext, globalObject);
	}

	KValueRef EvaluateFile(JSContextRef jsContext, const std::string& fullPath)
	{
		GetLogger()->Debug("Evaluating JavaScript file at: %s", fullPath.c_str());
		std::string scriptContents(FileUtils::ReadFile(fullPath));
		std::string fileURL(URLUtils::PathToFileURL(fullPath));
		return Evaluate(jsContext, scriptContents.c_str(), fileURL.c_str());
	}

	//===========================================================================//
	// METHODS BORROWED ARE TAKEN FROM GWT - modifications under same license
	//===========================================================================//
	/*
	 * Copyright 2008 Google Inc.
	 * 
	 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
	 * use this file except in compliance with the License. You may obtain a copy of
	 * the License at
	 * 
	 * http://www.apache.org/licenses/LICENSE-2.0
	 * 
	 * Unless required by applicable law or agreed to in writing, software
	 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
	 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
	 * License for the specific language governing permissions and limitations under
	 * the License.
	 */
	
	/*
	 * The following takes the prototype from the Function constructor, this allows
	 * us to easily support call and apply on our objects that support CallAsFunction.
	 *
	 * NOTE: The return value is not protected.
	 */
	static JSValueRef GetFunctionPrototype(JSContextRef jsContext, JSValueRef* exception) 
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef fnPropName = JSStringCreateWithUTF8CString("Function");
		JSValueRef fnCtorValue = JSObjectGetProperty(jsContext, globalObject,
			fnPropName, exception);
		JSStringRelease(fnPropName);
		if (!fnCtorValue)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSObjectRef fnCtorObject = JSValueToObject(jsContext, fnCtorValue, exception);
		if (!fnCtorObject)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSStringRef protoPropName = JSStringCreateWithUTF8CString("prototype");
		JSValueRef fnPrototype = JSObjectGetProperty(jsContext, fnCtorObject,
			protoPropName, exception);
		JSStringRelease(protoPropName);
		if (!fnPrototype)
		{
			return JSValueMakeUndefined(jsContext);
		}

	return fnPrototype;
	}

	/*
	 * The following takes the prototype from the Array constructor, this allows
	 * us to easily support array-like functions
	 *
	 * NOTE: The return value is not protected.
	 */
	static JSValueRef GetArrayPrototype(JSContextRef jsContext, JSValueRef* exception) 
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef fnPropName = JSStringCreateWithUTF8CString("Array");
		JSValueRef fnCtorValue = JSObjectGetProperty(jsContext, globalObject,
			fnPropName, exception);
		JSStringRelease(fnPropName);
		if (!fnCtorValue) 
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSObjectRef fnCtorObject = JSValueToObject(jsContext, fnCtorValue, exception);
		if (!fnCtorObject)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSStringRef protoPropName = JSStringCreateWithUTF8CString("prototype");
		JSValueRef fnPrototype = JSObjectGetProperty(jsContext, fnCtorObject,
			protoPropName, exception);
		JSStringRelease(protoPropName);
		if (!fnPrototype) 
		{
			return JSValueMakeUndefined(jsContext);
		}

		return fnPrototype;
	}

	KValueRef GetProperty(JSObjectRef globalObject, std::string name)
	{
		JSContextRef jsContext = GetGlobalContext(globalObject, NULL);
		JSStringRef jsName = JSStringCreateWithUTF8CString(name.c_str());
		JSValueRef prop = JSObjectGetProperty(jsContext, globalObject, jsName, NULL);
		JSStringRelease(jsName);
		return ToKrollValue(prop, jsContext, globalObject);
	}
}
}
