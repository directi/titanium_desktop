/**
 * @author Mital Vora<mital.d.vora@gmail.com>
 **/

#include "md5_utils.h"

#include <iomanip>
#include <sstream>
using namespace std;
#include <openssl/evp.h>


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
	}
}
