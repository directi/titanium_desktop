/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _SOCKET_MODULE_H_
#define _SOCKET_MODULE_H_

#include "socket_binding.h"
#include <curl/curl.h>

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define TITANIUM_SOCKET_API EXPORT
#elif defined(OS_WIN32)
# ifdef TITANIUM_SOCKET_API_EXPORT
#  define TITANIUM_SOCKET_API __declspec(dllexport)
# else
#  define TITANIUM_SOCKET_API __declspec(dllimport)
# endif
# define EXPORT __declspec(dllexport)
#endif


namespace ti 
{
	class TITANIUM_SOCKET_API SocketModule : public kroll::Module
	{
		KROLL_MODULE_CLASS(SocketModule)

	public:
		static std::string GetRootCertPath();
		static CURLSH* GetCurlShareHandle();

	private:
		AutoPtr<SocketBinding> socketBinding;
	};

}
#endif
