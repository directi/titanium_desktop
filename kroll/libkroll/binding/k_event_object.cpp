/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "../host.h"

#include "value.h"
#include "arg_list.h"
#include "global_object.h"
#include "k_event_object.h"
#include "value_exception.h"
#include "../javascript/k_kjs_method.h"
#include <kroll/MainThreadUtils.h>

namespace kroll
{
	KEventObject::KEventObject(const char *type) :
		KAccessorObject(type)
	{
		this->SetMethod("on", &KEventObject::_AddEventListener);
		this->SetMethod("addEventListener", &KEventObject::_AddEventListener);
		this->SetMethod("removeEventListener", &KEventObject::_RemoveEventListener);

		Event::SetEventConstants(this);
	}

	KEventObject::~KEventObject()
	{
		this->RemoveAllEventListeners();
	}

	AutoPtr<Event> KEventObject::CreateEvent(const std::string& eventName)
	{
		return new Event(AutoPtr<KEventObject>(this, true), eventName);
	}

	void KEventObject::AddEventListener(const std::string& event, KValueRef callback)
	{
		ASSERT_MAIN_THREAD
		listeners.push_back(new EventListener(event, callback));
	}

	void KEventObject::RemoveEventListener(const std::string& event, KValueRef callback)
	{
		ASSERT_MAIN_THREAD
		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			EventListener* listener = *i;
			if (listener->handles(event)
				&& listener->callback_is(callback))
			{
				this->listeners.erase(i);
				delete listener;
				break;
			}
			i++;
		}
	}

	void KEventObject::RemoveAllEventListeners()
	{
		ASSERT_MAIN_THREAD
		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			delete *i++;
		}

		this->listeners.clear();
	}

	void KEventObject::FireEvent(const std::string& event, const ValueList& args)
	{
		// Make a copy of the listeners map here, because firing the event might
		// take a while and we don't want to block other threads that just need
		// too add event listeners.
		ASSERT_MAIN_THREAD
		EventListenerList listenersCopy;
		listenersCopy = listeners;

		EventListenerList::iterator li = listenersCopy.begin();
		while (li != listenersCopy.end())
		{
			EventListener* listener = *li++;
			if (listener->handles(event))
			{
				try
				{
					if (!listener->dispatch(args))
					{
						// Stop event dispatch if callback tells us
						break;
					}
				}
				catch (ValueException& e)
				{
					this->ReportDispatchError(e.ToString());
					break;
				}
			}
		}	
	}

	bool KEventObject::FireEvent(const std::string& eventName)
	{
		AutoPtr<Event> event(this->CreateEvent(eventName));
		return this->FireEvent(event);
	}

	bool KEventObject::FireEvent(AutoPtr<Event> event)
	{
		// Make a copy of the listeners map here, because firing the event might
		// take a while and we don't want to block other threads that just need
		// too add event listeners.
		ASSERT_MAIN_THREAD
		EventListenerList listenersCopy;
		listenersCopy = listeners;

		EventListenerList::iterator li = listenersCopy.begin();
		while (li != listenersCopy.end())
		{
			EventListener* listener = *li++;
			if (listener->handles(event->getEventName()))
			{
				ValueList args(Value::NewObject(event));
				bool result = false;

				try
				{
					result = listener->dispatch(args);
				}
				catch (ValueException& e)
				{
					this->ReportDispatchError(e.ToString());
				}

				if (event->isStopped() || !result)
					return !event->isPreventedDefault();
			}
		}

		if (this != GlobalObject::GetInstance().get())
			GlobalObject::GetInstance()->FireEvent(event);

		return !event->isPreventedDefault();
	}

	void KEventObject::_AddEventListener(const ValueList& args, KValueRef result)
	{
		std::string event;
		KValueRef callback;

		if (args.size() == 2 && args.at(0)->IsString() && args.at(1)->IsMethod())
		{
			event = args.GetString(0);
			callback = args.GetValue(1);
		}
		else if (args.size() == 1 && args.at(0)->IsMethod())
		{
			event = Event::ALL;
			callback = args.GetValue(0);
		}
		else
		{
			throw ValueException::FromString("Incorrect arguments passed to addEventListener");
		}

		this->AddEventListener(event, callback);
		result->SetMethod(callback->ToMethod());
	}

	void KEventObject::_RemoveEventListener(const ValueList& args, KValueRef result)
	{
		args.VerifyException("removeEventListener", "s m");

		const std::string event(args.GetString(0));
		this->RemoveEventListener(event, args.GetValue(1));
	}

	void KEventObject::ReportDispatchError(std::string& reason)
	{
		this->logger()->Error("Failed to fire event: target=%s reason=%s",
			this->GetType().c_str(), reason.c_str());
	}

	EventListener::EventListener(const std::string& targetedEvent, KValueRef callback) :
		targetedEvent(targetedEvent),
		callback(callback)
	{
	}

	inline bool EventListener::handles(const std::string& ev) const
	{
		return targetedEvent == ev || targetedEvent == Event::ALL;
	}

	inline bool EventListener::callback_is(KValueRef callback) const
	{
		KMethodRef this_method = this->callback->ToMethod();
		if(this_method.isNull()) {
			Logger::Get("Event")->Warn("Event wasn't cleaned up");
			return false;
		}

		KMethodRef callback_method = callback->ToMethod();
		return (this_method->Equals(callback_method));
	}

	bool EventListener::dispatch(const ValueList& args)
	{
		if(this->callback->ToObject().isNull()) {
			Logger::Get("Event")->Warn("Event wasn't cleaned up");
			return true;
		}
		KValueRef result = RunOnMainThread(this->callback->ToMethod(), args);
		if (result->IsBool())
			return result->ToBool();
		return true;
	}
}

