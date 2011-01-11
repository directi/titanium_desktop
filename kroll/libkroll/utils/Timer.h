/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include <map>

#include <base.h>

#include <kroll/binding/binding.h>


namespace UTILS_NS
{
	class KROLL_API Timer
	{
		static unsigned timerid;

	protected:
		const unsigned id;
		const long duration;
		const bool recursive;

	public:
		Timer(long _duration, bool _recursive);
		virtual ~Timer() {}

		virtual void start()=0;
		virtual bool stop()=0;
		virtual void callback() = 0;

		unsigned getID() const { return id; }
	};

#ifdef OS_WIN32

	class KROLL_API Win32Timer : public Timer
	{
	private:
		unsigned loadTimeTimerID;

	public:
		Win32Timer(long _duration, bool _recursive);
		virtual ~Win32Timer();

		virtual void start();
		virtual bool stop();
	};

	class KROLL_API Win32KMethodCallerTimer
		: public Win32Timer
	{
	private:
		KMethodRef method;
		ValueList args;

	public:
		Win32KMethodCallerTimer(long _duration, bool _recursive, KMethodRef _method, ValueList& _args);
		virtual ~Win32KMethodCallerTimer();
		virtual void callback();
	};
#endif
}
#endif // _TIMER_H_