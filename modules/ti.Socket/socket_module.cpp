/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#include "socket_module.h"

#ifdef OS_WIN32
// TODO: This is poco UnWindows.h's curse.... to be removed with poco
#ifdef UNICODE
#define CreateEvent  CreateEventW
#else
#define CreateEvent  CreateEventA
#endif // !UNICODE
#endif

#include "SocketService.h"

using namespace kroll;

namespace ti
{
	KROLL_MODULE(SocketModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	void SocketModule::Initialize()
	{
		SocketService::initialize();
		this->socketBinding = new SocketBinding(host);
		GlobalObject::GetInstance()->SetObject("Socket", this->socketBinding);
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
}
