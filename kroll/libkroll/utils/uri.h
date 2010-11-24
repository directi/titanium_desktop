/**
 * @author Mital Vora <mital.d.vora@gmail.com>
 */
#ifndef _URI_H_
#define _URI_H_
#include <string>

#include <base.h>

namespace UTILS_NS
{
	class KROLL_API URI
	{
		bool valid;
		std::string scheme;
		std::string host;
		std::string path;

		void parse(const std::string &url);
		std::string parseScheme(std::string::const_iterator oIter,
			std::string::const_iterator end);
		void parseAuthority(std::string::const_iterator& oIter,
			const std::string::const_iterator& end);
	public:
		URI(const std::string &_uri);
		~URI();
		std::string getScheme() const;
		std::string getHost() const;
		std::string getPath() const;
	};
}
#endif
