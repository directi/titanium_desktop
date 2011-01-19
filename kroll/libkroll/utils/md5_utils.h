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
		KROLL_API std::string GetHexRepresentation(const unsigned char * data, size_t length);
		KROLL_API std::string calculate_md5_of(const void *content, size_t len);
		KROLL_API std::string calculate_md5_of(const std::string & data);
		KROLL_API std::string calculate_md5_of_file(const std::string & filename);
	}
}
#endif
