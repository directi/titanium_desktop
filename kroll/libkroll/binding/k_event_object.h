/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009-2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_EVENT_OBJECT_H_
#define _KR_EVENT_OBJECT_H_

#include <list>
#include <base.h>

#include "event.h"

namespace kroll
{
	class EventListener;
	typedef std::list<EventListener*> EventListenerList;

	class KROLL_API KEventObject : public KAccessorObject
	{
	public:
		KEventObject(const char* name = "KEventObject");
		virtual ~KEventObject();

		AutoPtr<Event> CreateEvent(const std::string& eventName);

		virtual void AddEventListener(const std::string& event, KValueRef listener);
		virtual void RemoveEventListener(const std::string& event, KValueRef listener);
		virtual void RemoveAllEventListeners();

		virtual void FireEvent(const std::string& event, const ValueList& args);
		virtual bool FireEvent(const std::string& event);
		virtual bool FireEvent(AutoPtr<Event>);

		void _AddEventListener(const ValueList&, KValueRef result);
		void _RemoveEventListener(const ValueList&, KValueRef result);
		void _RemoveAllEventListeners(const ValueList&, KValueRef result);

	private:
		void ReportDispatchError(const std::string& reason);

		EventListenerList listeners;
	};

	class EventListener 
	{
	public:
		EventListener(const std::string& targetedEvent, KValueRef callback);

		bool handles(const std::string& ev) const;
		bool dispatch(const ValueList& args);
		bool callback_is(KValueRef callback) const;

	private:
		const std::string targetedEvent;
		KValueRef callback;
	};
}

#endif
