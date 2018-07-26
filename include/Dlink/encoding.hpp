#ifndef DLINK_HEADER_ENCODING_HPP
#define DLINK_HEADER_ENCODING_HPP

#include <istream>
#include <string_view>

namespace dlink
{
	enum class encoding
	{
		none,

		utf8,
		utf16,
		utf16be,
		utf32,
		utf32be,
	};

	std::string to_string(dlink::encoding encoding);
	encoding detect_encoding(std::istream& stream);
}

#endif