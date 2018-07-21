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

	constexpr std::string_view to_string(dlink::encoding encoding) noexcept
	{
		switch (encoding)
		{
		case encoding::utf8:
			return "UTF-8";

		case encoding::utf16:
			return "UTF-16";

		case encoding::utf16be:
			return "UTF-16BE";

		case encoding::utf32:
			return "UTF-32";

		case encoding::utf32be:
			return "UTF-32BE";

		default:
			return "";
		}
	}
	encoding detect_encoding(std::istream& stream);
}

#endif