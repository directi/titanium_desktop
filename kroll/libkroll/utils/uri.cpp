/**
* @author Mital Vora <mital.d.vora@gmail.com>
*/

#include "uri.h"

#include <sstream>

namespace UTILS_NS
{
	URI::URI(const std::string &url)
		: valid(true),scheme("")
	{
		parse(url);
	}
	URI::~URI()
	{
	}

	void URI::parse(const std::string &url)
	{
		if(url.empty())
		{
			valid = false;
			return;
		}

		size_t sp = url.find_first_of( ':');
		if ( sp != std::string::npos )
		{
			this->scheme = std::string(url.begin(), url.begin()+sp);
		}

		size_t sp1 = url.find_first_of( '/', sp+3 /* skip "://" part */ );
		// TODO: fix this for file:/// url and other urls like "C:\"
		if ( sp1 != std::string::npos )
		{
			this->host = std::string(url.begin() + sp +3, url.begin() + sp1);
			size_t sp2 = url.find_last_of( '/');
			if ( sp2 != std::string::npos && sp1 != sp2)
			{
				this->path = std::string(url.begin() + sp1, url.begin() + sp2);
				// TODO: find out query here
			}
			else
			{
				this->path = std::string(url.begin() + sp1 + 1, url.end());
				// TODO: findout path bits... 
			}
		}
		else
		{
			this->host = std::string(url.begin() + sp +3, url.end());
		}
	}

	inline std::string URI::getScheme() const
	{
		return this->scheme;
	}
	inline std::string URI::getHost() const
	{
		return this->host;
	}
	inline std::string URI::getPath() const
	{
		return this->path;
	}
}
