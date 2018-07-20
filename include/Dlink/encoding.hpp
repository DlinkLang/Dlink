#ifndef DLINK_HEADER_ENCODING_HPP
#define DLINK_HEADER_ENCODING_HPP

#include <istream>

namespace dlink
{
	enum class encoding_type
	{
		none,

		utf8,
		utf16,
		utf16be,
		utf32,
		utf32be,
	};

	encoding_type detect_encoding(std::istream& stream);
}

#endif