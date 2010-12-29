/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#include "Timer.h"
#include <kroll/host.h>
#include "../Assertion.h"

namespace UTILS_NS
{
	unsigned Timer::timerid = 0;

	Timer::Timer(long _duration, bool _recursive)
		: id(++Timer::timerid),
		duration(_duration),
		recursive(_recursive)
	{
	}

#ifdef OS_WIN32

	std::map<unsigned, Timer *> timers;

	Win32Timer::Win32Timer(long _duration, bool _recursive)
		: Timer(_duration, _recursive),
		loadTimeTimerID(0)
	{
	}

	Win32Timer::~Win32Timer()
	{
		if (this->loadTimeTimerID != 0)
		{
			std::map<unsigned, Timer *>::iterator oIter = timers.find(this->loadTimeTimerID);
			if(oIter != timers.end())
			{
				timers.erase(oIter);
			}
		}
	}

	VOID CALLBACK TimerProc(HWND hwnd,
		UINT uMsg,	UINT_PTR idEvent, DWORD dwTime)
	{
			std::map<unsigned, Timer *>::iterator oIter = timers.find(idEvent);
			if(oIter != timers.end())
			{
				Timer * timer = oIter->second;
				timer->callback();
			}
	}


	void Win32Timer::start()
	{
		this->loadTimeTimerID = SetTimer(NULL, this->id, this->duration, &TimerProc);
		timers[this->loadTimeTimerID] = this;
	}

	bool Win32Timer::stop()
	{
		if (this->loadTimeTimerID)
		{
			KillTimer(0 , this->loadTimeTimerID);
			std::map<unsigned, Timer *>::iterator oIter = timers.find(this->loadTimeTimerID);
			if(oIter != timers.end())
			{
				timers.erase(oIter);
				return true;
			}
		}
		return false;
	}

	Win32KMethodCallerTimer::Win32KMethodCallerTimer(long _duration,
		bool _recursive,
		KMethodRef _method,
		ValueList& _args)
		: Win32Timer(_duration, _recursive),
		method(_method),
		args(_args)
	{
	}

	Win32KMethodCallerTimer::~Win32KMethodCallerTimer()
	{
	}

	void Win32KMethodCallerTimer::callback()
	{
		ASSERT_MAIN_THREAD
		if (! this->recursive)
		{
			this->stop();
		}
		method->Call(args);
	}

#endif
}
