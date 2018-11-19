#ifndef DLINK_HEADER_ENCODING_HPP
#define DLINK_HEADER_ENCODING_HPP

#include <istream>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

	int get_character_length(char first_byte);

	enum class eol
	{
		lf,				// Line Feed,			0x0A
#ifndef DLINK_LEAN_AND_MEAN
		vt,				// Vertical Tab,		0x0B
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
	bool is_eol(std::istream& stream, int& char_size);

	enum class whitespace
	{
		tab,						// Character Tabulation,	0x09
		line_feed,					//							0x0A,	EOL
#ifndef DLINK_LEAN_AND_MEAN
		line_tabulation,			//							0x0B,	EOL
		form_feed,					//							0x0C,	EOL
#endif
		carriage_return,			//							0x0D,	EOL
		carriage_return_line_feed,	// Carriage Return + Line Feed
		space,						//							0x20
#ifndef DLINK_LEAN_AND_MEAN
		next_line,					//							0x85,	EOL
		no_break_space,				//							0xA0
		ogham_space_mark,			//							0x1680
		en_quad,					//							0x2000
		em_quad,					//							0x2001
		en_space,					//							0x2002
		em_space,					//							0x2003
		three_per_em_space,			//							0x2004
		four_per_em_space,			//							0x2005
		six_per_em_space,			//							0x2006
		figure_space,				//							0x2007
		punctuation_space,			//							0x2008
		thin_space,					//							0x2009
		hair_space,					//							0x200A
		line_separator,				//							0x2028,	EOL
		paragraph_separator,		//							0x2029,	EOL
		narrow_no_break_space,		//							0x202F
		medium_mathematical_space,	//							0x205F
#endif
		ideographic_space,			//							0x3000
	};

	extern const std::map<std::string_view, whitespace> whitespaces;

	std::string_view get_whitespace_character(dlink::whitespace whitespace);
	std::string to_string(dlink::whitespace whitespace);
	bool is_whitespace(std::istream& stream);
	bool is_whitespace(std::istream& stream, char& output);
	bool is_whitespace(std::istream& stream, char& output, whitespace& type);
	std::pair<std::string, std::vector<std::pair<std::size_t, std::size_t>>> replace_with_space(const std::string_view& string);

	bool isdigit(char c) noexcept;
	bool isalpha(char c) noexcept;
}

#endif