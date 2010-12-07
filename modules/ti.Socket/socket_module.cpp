/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#include "socket_module.h"
#include "SocketService.h"
#include <boost/thread/mutex.hpp> 

using namespace kroll;

namespace ti
{
	KROLL_MODULE(SocketModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));
	static CURLSH* curlShareHandle = 0;

	static boost::mutex * SharedResourceMutex(curl_lock_data data)
	{
		static boost::mutex cookieMutex;
		static boost::mutex dnsMutex;
		static boost::mutex shareMutex;

		switch (data)
		{
			case CURL_LOCK_DATA_COOKIE:
				return &cookieMutex;
			case CURL_LOCK_DATA_DNS:
				return &dnsMutex;
			case CURL_LOCK_DATA_SHARE:
				return &shareMutex;
			default:
				return NULL;
		}
	}


	static void CurlLockCallback(CURL* handle, curl_lock_data data, curl_lock_access, void*)
	{
		if (boost::mutex* mutex = SharedResourceMutex(data))
			mutex->lock();
	}

	static void CurlUnlockCallback(CURL* handle, curl_lock_data data, void* userPtr)
	{
		if (boost::mutex* mutex = SharedResourceMutex(data))
			mutex->unlock();
	}


	void SocketModule::Initialize()
	{
		SocketService::initialize();
		this->socketBinding = new SocketBinding(host);
		GlobalObject::GetInstance()->SetObject("Socket", this->socketBinding);

		curlShareHandle = curl_share_init();
		curl_share_setopt(curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
		curl_share_setopt(curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
		curl_share_setopt(curlShareHandle, CURLSHOPT_LOCKFUNC, CurlLockCallback);
		curl_share_setopt(curlShareHandle, CURLSHOPT_UNLOCKFUNC, CurlUnlockCallback);

	}

	void SocketModule::Stop()
	{
		SocketService::uninitialize();
	}

	std::string SocketModule::GetRootCertPath()
	{
		std::string path;
		SharedApplication app(Host::GetInstance()->GetApplication());
		path = FileUtils::Join(app->getRuntimePath().c_str(), "rootcert.pem", 0);
		return path;
	}

	CURLSH* SocketModule::GetCurlShareHandle()
	{
		return curlShareHandle;
	}

}
