#include <Dlink/encoding.hpp>

#include <algorithm>
#include <cctype>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

namespace dlink
{
	std::string to_string(dlink::encoding encoding)
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
	encoding detect_encoding(std::istream& stream)
	{
		static const std::uint8_t utf8_bom[] = { 0xEF, 0xBB, 0xBF };
		static const std::uint8_t utf16_bom[] = { 0xFF, 0xFE };
		static const std::uint8_t utf16be_bom[] = { 0xFE, 0xFF };
		static const std::uint8_t utf32_bom[] = { 0xFF, 0xFE, 0x00, 0x00 };
		static const std::uint8_t utf32be_bom[] = { 0x00, 0x00, 0xFE, 0xFF };
		
		const std::streampos pos = stream.tellg();

		std::uint8_t bom_buffer[4];
		stream.read(reinterpret_cast<char*>(bom_buffer), 4);

		const std::streamsize read_count = stream.gcount();
		if (read_count == 0)
		{
			return encoding::none;
		}

		if (read_count == 4)
		{
			if (std::equal(bom_buffer, bom_buffer + 4, utf32_bom))
			{
				return encoding::utf32;
			}
			else if (std::equal(bom_buffer, bom_buffer + 4, utf32be_bom))
			{
				return encoding::utf32be;
			}
		}
		if (read_count >= 3 && std::equal(bom_buffer, bom_buffer + 3, utf8_bom))
		{
			stream.clear();
			stream.seekg(pos + static_cast<std::streamoff>(3), std::ios::beg);
			return encoding::utf8;
		}
		if (read_count >= 2)
		{
			if (std::equal(bom_buffer, bom_buffer + 2, utf16_bom))
			{
				stream.clear();
				stream.seekg(pos + static_cast<std::streamoff>(2), std::ios::beg);
				return encoding::utf16;
			}
			else if (std::equal(bom_buffer, bom_buffer + 2, utf16be_bom))
			{
				stream.clear();
				stream.seekg(pos + static_cast<std::streamoff>(2), std::ios::beg);
				return encoding::utf16be;
			}
		}

		stream.clear();
		stream.seekg(pos, std::ios::beg);
		return encoding::none;
	}

	int get_character_length(char first_byte)
	{
		const unsigned char uc = static_cast<unsigned char>(first_byte);

		if (uc < 0x80)
		{
			return 1;
		}
		else if ((uc & 0xF0) == 0xF0)
		{
			return 4;
		}
		else if ((uc & 0xE0) == 0xE0)
		{
			return 3;
		}
		else if ((uc & 0xC0) == 0xC0)
		{
			return 2;
		}

		throw std::invalid_argument("The argument 'first_byte' isn't the first byte of a valid Unicode character encoded in UTF-8.");
	}
}

namespace dlink
{
	const std::map<std::string_view, eol> eols =
	{
		{ "\u000A", eol::lf },
#ifndef DLINK_LEAN_AND_MEAN
		{ "\u000B", eol::vt },
		{ "\u000C", eol::ff },
#endif
		{ "\u000D", eol::cr },
		{ "\u000D\u000A", eol::crlf },
#ifndef DLINK_LEAN_AND_MEAN
		{ u8"\u0085", eol::nel },
		{ u8"\u2028", eol::ls },
		{ u8"\u2029", eol::ps },
		{ "\u001E", eol::rs }
#endif
	};

	std::string_view get_eol_character(dlink::eol eol)
	{
		return std::find_if(eols.begin(), eols.end(), [eol](const std::pair<std::string_view, dlink::eol>& pair)
		{
			return pair.second == eol;
		})->first;
	}
	std::string to_string(dlink::eol eol)
	{
		switch (eol)
		{
		case dlink::eol::lf:
			return "LF";

#ifndef DLINK_LEAN_AND_MEAN
		case dlink::eol::vt:
			return "VT";

		case dlink::eol::ff:
			return "FF";
#endif

		case dlink::eol::cr:
			return "CR";

		case dlink::eol::crlf:
			return "CR+LF";

#ifndef DLINK_LEAN_AND_MEAN
		case dlink::eol::nel:
			return "NEL";

		case dlink::eol::ls:
			return "LS";

		case dlink::eol::ps:
			return "PS";

		case dlink::eol::rs:
			return "RS";
#endif

		default:
			return "";
		}
	}
	bool is_eol(std::istream& stream, int& char_size)
	{
		char c;
		stream.read(&c, 1);

		if (!stream.good())
			return true;

		const int length = get_character_length(c);
		char_size = length;

#ifdef DLINK_LEAN_AND_MEAN
		if (length > 1)
#else
		if (length == 4)
#endif
		{
			stream.seekg(-1, std::ios::cur);
			return false;
		}

#ifndef DLINK_LEAN_AND_MEAN
		if (length == 1)
		{
#endif
			if (eols.find(std::string_view(&c, 1)) != eols.end())
			{
				stream.read(&c, 1);

				if (c != '\u000A')
				{
					stream.seekg(-1, std::ios::cur);
				}

				return true;
			}
#ifndef DLINK_LEAN_AND_MEAN
		}
		else
		{
			char c_array[3] = { c, 0, 0 };
			stream.read(c_array + 1, length - 1);

			if (eols.find(std::string_view(c_array, length)) != eols.end())
			{
				return true;
			}
		}
#endif

		stream.seekg(-length, std::ios::cur);
		return false;
	}
}

namespace dlink
{
	const std::map<std::string_view, whitespace> whitespaces =
	{
		{ "\u0009", whitespace::tab },
		{ "\u000A", whitespace::line_feed },
#ifndef DLINK_LEAN_AND_MEAN
		{ "\u000B", whitespace::line_tabulation },
		{ "\u000C", whitespace::form_feed },
#endif
		{ "\u000D", whitespace::carriage_return },
		{ "\u000D\u000A", whitespace::carriage_return_line_feed },
		{ "\u0020", whitespace::space },
#ifndef DLINK_LEAN_AND_MEAN
		{ u8"\u0085", whitespace::next_line },
		{ u8"\u00A0", whitespace::no_break_space },
		{ u8"\u1680", whitespace::ogham_space_mark },
		{ u8"\u2000", whitespace::en_quad },
		{ u8"\u2001", whitespace::em_quad },
		{ u8"\u2002", whitespace::en_space },
		{ u8"\u2003", whitespace::em_space },
		{ u8"\u2004", whitespace::three_per_em_space },
		{ u8"\u2005", whitespace::four_per_em_space },
		{ u8"\u2006", whitespace::six_per_em_space },
		{ u8"\u2007", whitespace::figure_space },
		{ u8"\u2008", whitespace::punctuation_space },
		{ u8"\u2009", whitespace::thin_space },
		{ u8"\u200A", whitespace::hair_space },
		{ u8"\u2028", whitespace::line_separator },
		{ u8"\u2029", whitespace::paragraph_separator },
		{ u8"\u202F", whitespace::narrow_no_break_space },
		{ u8"\u205F", whitespace::medium_mathematical_space },
#endif
		{ u8"\u3000", whitespace::ideographic_space },
	};

	std::string_view get_whitespace_character(dlink::whitespace whitespace)
	{
		return std::find_if(whitespaces.begin(), whitespaces.end(), [whitespace](const std::pair<std::string_view, dlink::whitespace>& pair)
		{
			return pair.second == whitespace;
		})->first;
	}
	std::string to_string(dlink::whitespace whitespace)
	{
		switch (whitespace)
		{
		case dlink::whitespace::tab:
			return "CHARACTER TABULATION";

		case dlink::whitespace::line_feed:
			return "LINE FEED";

#ifndef DLINK_LEAN_AND_MEAN
		case dlink::whitespace::line_tabulation:
			return "LINE TABULATION";

		case dlink::whitespace::form_feed:
			return "FORM FEED";
#endif

		case dlink::whitespace::carriage_return:
			return "CARRIAGE RETURN";

		case dlink::whitespace::carriage_return_line_feed:
			return "CARRIAGE RETURN + LINE FEED";

		case dlink::whitespace::space:
			return "SPACE";

#ifndef DLINK_LEAN_AND_MEAN
		case dlink::whitespace::next_line:
			return "NEXT LINE";

		case dlink::whitespace::no_break_space:
			return "NO-BREAK SPACE";

		case dlink::whitespace::ogham_space_mark:
			return "OGHAM SPACE MARK";

		case dlink::whitespace::en_quad:
			return "EN QUAD";

		case dlink::whitespace::em_quad:
			return "EM QUAD";

		case dlink::whitespace::three_per_em_space:
			return "THREE-PER-EM SPACE";

		case dlink::whitespace::four_per_em_space:
			return "FOUR-PER-EM SPACE";

		case dlink::whitespace::six_per_em_space:
			return "SIX-PER-EM SPACE";

		case dlink::whitespace::figure_space:
			return "FIGURE SPACE";

		case dlink::whitespace::punctuation_space:
			return "PUNCTUATION SPACE";

		case dlink::whitespace::thin_space:
			return "THIN SPACE";

		case dlink::whitespace::hair_space:
			return "HAIR SPACE";

		case dlink::whitespace::line_separator:
			return "LINE SEPARATOR";

		case dlink::whitespace::paragraph_separator:
			return "PARAGRAPH SEPARATOR";

		case dlink::whitespace::narrow_no_break_space:
			return "NARROW NO-BREAK SPACE";

		case dlink::whitespace::medium_mathematical_space:
			return "MEDIUM MATHMATICAL SPACE";
#endif

		case dlink::whitespace::ideographic_space:
			return "IDEOGRAPHIC SPACE";

		default:
			return "";
		}
	}
	bool is_whitespace(std::istream& stream)
	{
		char dummy_output;
		whitespace dummy_type;

		const bool result = is_whitespace(stream, dummy_output, dummy_type);
		if (result) return true;

		const int dummy_output_size = get_character_length(dummy_output);
		stream.seekg(dummy_output_size);

		return false;
	}
	bool is_whitespace(std::istream& stream, char& output)
	{
		whitespace dummy;
		return is_whitespace(stream, output, dummy);
	}
	bool is_whitespace(std::istream& stream, char& output, whitespace& type)
	{
		char c;
		stream.read(&c, 1);
		if (!stream.good()) return true;

		const int length = get_character_length(c);
		output = c;

		if (length == 4)
		{
			output = c;
			stream.seekg(3, std::ios::cur);
			return false;
		}
		
		if (length == 1)
		{
			if (std::map<std::string_view, whitespace>::const_iterator iter = whitespaces.find(std::string_view(&c, 1));
				iter != whitespaces.end())
			{
				type = iter->second;

				const char prev_c = c;

				stream.read(&c, 1);
				if (!stream.good()) return true;

				if (prev_c == '\u000D' && c == '\u000A')
				{
					type = whitespace::carriage_return_line_feed;
				}
				else
				{
					stream.seekg(-1, std::ios::cur);
				}

				return true;
			}
		}
		else
		{
			char c_array[3] = { c, 0, 0 };
			stream.read(c_array + 1, length - 1);

			if (std::map<std::string_view, whitespace>::const_iterator iter = whitespaces.find(std::string_view(c_array, length));
				iter != whitespaces.end())
			{
				type = iter->second;
				return true;
			}
		}

		output = c;
		return false;
	}
	std::pair<std::string, std::vector<std::pair<std::size_t, std::size_t>>> replace_with_space(const std::string_view& string)
	{
		static const std::map<dlink::whitespace, std::string_view> map =
		{
			{ dlink::whitespace::tab, "    " },
			{ dlink::whitespace::space, " " },
#ifndef DLINK_LEAN_AND_MEAN
			{ dlink::whitespace::no_break_space, " " },
			{ dlink::whitespace::ogham_space_mark, " " },
			{ dlink::whitespace::en_quad, " " },
			{ dlink::whitespace::em_quad, " " },
			{ dlink::whitespace::en_space, " " },
			{ dlink::whitespace::em_space, " " },
			{ dlink::whitespace::three_per_em_space, " " },
			{ dlink::whitespace::four_per_em_space, " " },
			{ dlink::whitespace::six_per_em_space, " " },
			{ dlink::whitespace::figure_space, " " },
			{ dlink::whitespace::punctuation_space, " " },
			{ dlink::whitespace::thin_space, " " },
			{ dlink::whitespace::hair_space, " " },
			{ dlink::whitespace::narrow_no_break_space, " " },
			{ dlink::whitespace::medium_mathematical_space, " " },
#endif
			{ dlink::whitespace::ideographic_space, " " },
		};

		std::string result;
		std::vector<std::pair<std::size_t, std::size_t>> replaced_pos;

		boost::iostreams::stream<boost::iostreams::basic_array_source<char>> stream(string.data(), string.size());

		std::size_t offset = 0;
		std::size_t length = 0;

		while (!stream.eof())
		{
			char c;
			whitespace whitespace_type;
			
			if (is_whitespace(stream, c, whitespace_type))
			{
				result += string.substr(offset, length);
				offset += length;
				length = 0;

				if (stream.eof()) break;

				if (const std::map<whitespace, std::string_view>::const_iterator iter = map.find(whitespace_type);
					iter != map.end())
				{
					result += iter->second;
					replaced_pos.push_back(std::make_pair(offset, iter->second.size()));
				}
				else
				{
					result += string.substr(offset, get_character_length(c));
				}

				offset += get_character_length(c);
			}
			else
			{
				length += get_character_length(c);
			}
		}

		return std::make_pair(std::move(result), std::move(replaced_pos));
	}

	bool isdigit(char c) noexcept
	{
		if (static_cast<unsigned char>(c) > 0x7F) return false;
		else return std::isdigit(c);
	}
	bool isalpha(char c) noexcept
	{
		if (static_cast<unsigned char>(c) > 0x7F) return false;
		else return std::isalpha(c);
	}
}