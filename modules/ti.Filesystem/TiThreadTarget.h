#ifndef _TI_FOUNDATION_THREADTARGET_H_
#define _TI_FOUNDATION_THREADTARGET_H_

#include <kroll/kroll.h>


namespace ti
{
	class TiThreadTarget
	{
	public:
		typedef void (*Callback)(void *);

		TiThreadTarget(Callback method, void * param);
		virtual ~TiThreadTarget();

		TiThreadTarget& operator = (const TiThreadTarget& te);

		void run();

	private:

		Callback _method;
		void * _param;
	};

	inline void TiThreadTarget::run()
	{
		_method(_param);
	}
}

#endif // _TI_FOUNDATION_THREADTARGET_H_