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

	std::string to_string(dlink::eol eol)
	{
		switch (eol)
		{
		case dlink::eol::lf:
			return "LF";

		case dlink::eol::ff:
			return "FF";

		case dlink::eol::cr:
			return "CR";

		case dlink::eol::crlf:
			return "CR+LF";

		case dlink::eol::nel:
			return "NEL";

		case dlink::eol::ls:
			return "LS";

		case dlink::eol::ps:
			return "PS";

		case dlink::eol::rs:
			return "RS";

		default:
			return "";
		}
	}
}