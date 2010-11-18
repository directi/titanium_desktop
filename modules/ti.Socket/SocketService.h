/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _SOCKET_SERVICE_H_
#define _SOCKET_SERVICE_H_

#include <kroll/kroll.h>

// TODO: This is poco UnWindows.h's curse.... to be removed with poco
#ifdef UNICODE
#define CreateEvent  CreateEventW
#else
#define CreateEvent  CreateEventA
#endif // !UNICODE

#include <string>
#include <deque>

#include <asio.hpp>
#include <asio/detail/mutex.hpp>

using asio::ip::tcp;

#include <boost/bind.hpp>

namespace ti
{
	class SocketService
	{
	public:
		static void initialize();
		static void uninitialize();
		static asio::io_service* getIOService()
		{
			return io_service.get();
		}

	private:
		static std::auto_ptr<asio::io_service> io_service;
		static std::auto_ptr<asio::io_service::work> io_idlework;
		static std::auto_ptr<asio::thread> io_thread;
	};
}
#endif
