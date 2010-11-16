/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009-2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_EVENT_OBJECT_H_
#define _KR_EVENT_OBJECT_H_

#include <list>
#include <Poco/Mutex.h>

#include <base.h>

#include "event.h"

#ifdef KROLL_API_EXPORT
#include <JavaScriptCore/JSContextRef.h>
#endif

namespace kroll
{
	class EventListener;
	typedef std::list<EventListener*> EventListenerList;

#ifdef KROLL_API_EXPORT
	class KEventObject;
	typedef std::list<KEventObject*> EventObjectList;
	typedef std::map<JSContextRef, EventObjectList*> ContextMap;
#endif

	class KROLL_API KEventObject : public KAccessorObject
	{
	public:
		KEventObject(const char* name = "KEventObject");
		virtual ~KEventObject();

		AutoPtr<Event> CreateEvent(const std::string& eventName);

		virtual void AddEventListener(std::string& event, KMethodRef listener);
		virtual void RemoveEventListener(std::string& event, KMethodRef listener);
		virtual void RemoveAllEventListeners();

		virtual void FireEvent(std::string& event, const ValueList& args);
		virtual bool FireEvent(std::string& event, bool synchronous=true);
		virtual bool FireEvent(AutoPtr<Event>, bool synchronous=true);

		void _AddEventListener(const ValueList&, KValueRef result);
		void _RemoveEventListener(const ValueList&, KValueRef result);
		void _RemoveAllEventListeners(const ValueList&, KValueRef result);

#ifdef KROLL_API_EXPORT
		static void CleanupListenersFromContext(JSContextRef context);
#endif

	private:
		void ReportDispatchError(std::string& reason);

		EventListenerList listeners;
		Poco::Mutex listenersMutex;

#ifdef KROLL_API_EXPORT
		static Poco::Mutex mapMutex;
		static ContextMap contextMap;

		static void AddRef(JSContextRef context, KEventObject* obj);
		static void DelRef(JSContextRef context, KEventObject* obj);
#endif
	};

	class EventListener 
	{
	public:
		EventListener(std::string& targetedEvent, KMethodRef callback);

		bool Handles(std::string& event);
		bool Dispatch(KObjectRef thisObject, const ValueList& args, bool synchronous);
		KMethodRef Callback();

	private:
		std::string targetedEvent;
		KMethodRef callback;
	};
}

#endif
