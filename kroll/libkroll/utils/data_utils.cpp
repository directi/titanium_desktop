
#include "data_utils.h"

#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace UTILS_NS
{
	namespace DataUtils
	{
		std::string GenerateUUID()
		{
			boost::uuids::random_generator generator;
			boost::uuids::uuid newuuid(generator());
			std::stringstream ss;
			ss << newuuid;
			return ss.str();
		}
	}
}
