#include "TiThreadTarget.h"


namespace ti
{
	TiThreadTarget::TiThreadTarget(Callback method, void * param)
		: _method(method),
		_param(param)
	{
	}

	TiThreadTarget& TiThreadTarget::operator = (const TiThreadTarget& te)
	{
		_method  = te._method;
		_param   = te._param;
		return *this;
	}

	TiThreadTarget::~TiThreadTarget()
	{
		if (_param)
		{
			delete _param;
		}
	}
}
