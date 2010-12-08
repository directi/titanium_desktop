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

		KMethodRef method;
		ValueList args;

	protected:
		const unsigned id;
		const long duration;
		const bool recursive;

	public:
		Timer(long _duration, bool _recursive, KMethodRef _method, ValueList& _args);
		virtual ~Timer() {}

		virtual void start()=0;
		virtual bool stop()=0;

		unsigned getID() const { return id; }
		void callback();
	};

#ifdef OS_WIN32

	class KROLL_API Win32Timer : public Timer
	{
	private:
		unsigned loadTimeTimerID;

	public:
		Win32Timer(long _duration, bool _recursive, KMethodRef _method, ValueList& _args);
		virtual ~Win32Timer();

		virtual void start();
		virtual bool stop();
	};
#endif
}
#endif // _TIMER_H_