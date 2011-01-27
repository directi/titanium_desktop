/**
 * @author Mital Vora<mital.d.vora@gmail.com>
 **/

#include "md5_utils.h"

#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;
#include <boost/array.hpp>
#include <openssl/evp.h>

class MD5Exception : public std::exception
{
	private:
		const std::string description;
	public:
		MD5Exception(const std::string & desc) : description(desc) {}
		virtual ~MD5Exception() throw() {}
		virtual std::string what() throw() { return description; }
};

namespace UTILS_NS
{
	namespace MD5Utils
	{
		std::string GetHexRepresentation(const unsigned char * data, size_t length)
		{
			std::ostringstream os;
			os.fill('0');
			os<<std::hex;
			for(const unsigned char * ptr=data;ptr<data+length;ptr++)
			{
				os<<std::setw(2)<<(unsigned int)*ptr;
			}
			return os.str();
		}

		std::string calculate_md5_of(const void *content, size_t len)
		{
			EVP_MD_CTX mdctx;
			unsigned char md_value[EVP_MAX_MD_SIZE];
			unsigned int md_len;

			EVP_DigestInit(&mdctx, EVP_md5());
			EVP_DigestUpdate(&mdctx, content, (size_t) len);
			EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
			EVP_MD_CTX_cleanup(&mdctx);
			return GetHexRepresentation(md_value, md_len);
		}

		std::string calculate_md5_of(const std::string & data)
		{
			return calculate_md5_of(data.c_str(), data.size());
		}

		std::string calculate_md5_of_file(const std::string & filename)
		{
			std::ifstream file(filename.c_str(), std::ios::binary);
			if (!file.is_open())
			{
				const std::string err("cannot open input file: " + filename);
				throw MD5Exception(err);
			}

			EVP_MD_CTX mdctx;
			EVP_DigestInit(&mdctx, EVP_md5());

			while ( !file.eof() && file.good() )
			{
				boost::array< char, 4096 > buffer;
				file.read(buffer.c_array(), buffer.size());
				EVP_DigestUpdate(&mdctx, buffer.data(), file.gcount());
			}

			unsigned char md_value[EVP_MAX_MD_SIZE];
			unsigned int md_len;
			EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
			EVP_MD_CTX_cleanup(&mdctx);
			return GetHexRepresentation(md_value, md_len);
		}
	}
}
