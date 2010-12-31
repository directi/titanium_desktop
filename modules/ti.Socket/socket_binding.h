/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _SOCKET_BINDING_H_
#define _SOCKET_BINDING_H_

#include <kroll/kroll.h>


namespace ti
{
	class SocketBinding : public KAccessorObject
	{
	public:
		SocketBinding(Host*);
		virtual ~SocketBinding();

		Host* GetHost();

	private:
		Host* host;
		KObjectRef global;

		void _CreateTCPSocket(const ValueList& args, KValueRef result);
		void _getSSLTCPSocket(const ValueList& args, KValueRef result);
		void _getCURLHTTPClient(const ValueList& args, KValueRef result);
	};
}

#endif
