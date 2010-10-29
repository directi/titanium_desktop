/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#include "socket_module.h"

using namespace kroll;

namespace ti
{
	KROLL_MODULE(SocketModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	void SocketModule::Initialize()
	{
		this->socketBinding = new SocketBinding(host);
		GlobalObject::GetInstance()->SetObject("Socket", this->socketBinding);
	}

	void SocketModule::Stop()
	{
	}

	std::string SocketModule::GetRootCertPath()
	{
		std::string path;
		SharedApplication app(Host::GetInstance()->GetApplication());
		path = FileUtils::Join(app->getRuntimePath().c_str(), "rootcert.pem", 0);
		return path;
	}
}
