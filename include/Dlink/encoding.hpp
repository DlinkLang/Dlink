#ifndef DLINK_HEADER_ENCODING_HPP
#define DLINK_HEADER_ENCODING_HPP

#include <istream>
#include <map>
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
#ifndef DLINK_LEAN_AND_MEAN
		ff,				// Form Feed,			0x0C
#endif
		cr,				// Carriage Return,		0x0D
		crlf,			// Carriage Return + Line Feed
#ifndef DLINK_LEAN_AND_MEAN
		nel,			// Next Line,			0x85
		ls,				// Line Separator,		0x2028
		ps,				// Paragraph Separator, 0x2029
		rs,				// Record Sperator,		0x1E
#endif
	};

	extern const std::map<std::string_view, eol> eols;

	std::string_view get_eol_character(dlink::eol eol);
	std::string to_string(dlink::eol eol);
	bool is_eol(std::istream& stream);
}

#endif