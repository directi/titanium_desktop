/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <base.h>
#include <data_utils.h>

#if defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
	#include <Poco/DigestEngine.h>
	#include <Poco/MD5Engine.h>

	using Poco::DigestEngine;
	using Poco::MD5Engine;
#else
	#include "poco/KDigestEngine.h"
	#include "poco/KMD5Engine.h"

	using KPoco::DigestEngine;
	using KPoco::MD5Engine;
#endif

namespace UTILS_NS
{
namespace DataUtils
{
	std::string HexMD5(std::string data)
	{
		MD5Engine engine;
		engine.update(data);
		return DigestEngine::digestToHex(engine.digest());
	}
}
}
