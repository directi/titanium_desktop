/**
 * @author Mital Vora<mital.d.vora@gmail.com>
 **/

#ifndef _MD5_UTILS_H_
#define _MD5_UTILS_H_

#include <base.h>

namespace UTILS_NS
{
	namespace MD5Utils
	{
		std::string getDigestForFile(const std::string& path);
	}
}
#endif
