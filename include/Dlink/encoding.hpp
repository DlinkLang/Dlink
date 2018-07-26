#ifndef DLINK_HEADER_ENCODING_HPP
#define DLINK_HEADER_ENCODING_HPP

#include <istream>
#include <string>
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

	enum class eol
	{
		lf,				// Line Feed,			0x0A
		ff,				// Form Feed,			0x0C
		cr,				// Carriage Return,		0x0D
		crlf,			// Carriage Return + Line Feed
		nel,			// Next Line,			0x85
		ls,				// Line Separator,		0x2028
		ps,				// Paragraph Separator, 0x2029
		rs,				// Record Sperator,		0x1E
	};

	constexpr std::string_view get_eol_character(dlink::eol eol) noexcept
	{
		switch (eol)
		{
		case dlink::eol::lf:
			return "\u000A";

		case dlink::eol::ff:
			return "\u000C";

		case dlink::eol::cr:
			return "\u000D";

		case dlink::eol::crlf:
			return "\u000D\u000A";

		case dlink::eol::nel:
			return "\u0085";

		case dlink::eol::ls:
			return "\u2028";

		case dlink::eol::ps:
			return "\u2029";

		case dlink::eol::rs:
			return "\u001E";

		default:
			return "";
		}
	}
	std::string to_string(dlink::eol eol);
}

#endif