#include <Dlink/encoding.hpp>

#include <algorithm>

namespace dlink
{
	encoding_type detect_encoding(std::istream& stream)
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
			return encoding_type::utf8;
		}
		
		stream.read(reinterpret_cast<char*>(bom_buffer) + 3, 1);
		
		if (std::equal(bom_buffer, bom_buffer + 4, utf32_bom))
		{
			return encoding_type::utf32;
		}
		else if (std::equal(bom_buffer, bom_buffer + 4, utf32be_bom))
		{
			return encoding_type::utf32be;
		}

		stream.seekg(-2, std::ios::cur);

		if (std::equal(bom_buffer, bom_buffer + 2, utf16_bom))
		{
			return encoding_type::utf16;
		}
		else if (std::equal(bom_buffer, bom_buffer + 2, utf16be_bom))
		{
			return encoding_type::utf16be;
		}

		stream.seekg(-2, std::ios::cur);
		return encoding_type::none;
	}
}