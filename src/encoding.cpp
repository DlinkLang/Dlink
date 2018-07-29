#include <Dlink/encoding.hpp>

#include <algorithm>

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

		std::uint8_t bom_buffer[4];
		stream.read(reinterpret_cast<char*>(bom_buffer), 3);

		if (std::equal(bom_buffer, bom_buffer + 3, utf8_bom))
		{
			return encoding::utf8;
		}

		stream.read(reinterpret_cast<char*>(bom_buffer) + 3, 1);

		if (std::equal(bom_buffer, bom_buffer + 4, utf32_bom))
		{
			return encoding::utf32;
		}
		else if (std::equal(bom_buffer, bom_buffer + 4, utf32be_bom))
		{
			return encoding::utf32be;
		}

		stream.seekg(-2, std::ios::cur);

		if (std::equal(bom_buffer, bom_buffer + 2, utf16_bom))
		{
			return encoding::utf16;
		}
		else if (std::equal(bom_buffer, bom_buffer + 2, utf16be_bom))
		{
			return encoding::utf16be;
		}

		stream.seekg(-2, std::ios::cur);
		return encoding::none;
	}
}

namespace dlink
{
	const std::map<std::string_view, eol> eols =
	{
		{ "\u000A", eol::lf },
#ifndef DLINK_LEAN_AND_MEAN
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
	bool is_eol(std::istream& stream)
	{
		char c;
		stream.read(&c, 1);

		if (!stream.good())
			return true;

		const unsigned char uc = static_cast<unsigned char>(c);
		const int length = [uc]()
		{
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
			else
			{
				return 2;
			}
		}();
		
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