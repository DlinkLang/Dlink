#ifndef DLINK_HEADER_ENCODING_HPP
#define DLINK_HEADER_ENCODING_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace dlink::encoding
{
	constexpr bool is_valid_unicode(char32_t character)
	{
		return character < 0x110000;
	}
	constexpr bool is_invalid_unicode(char32_t character)
	{
		return character >= 0x110000;
	}

	class utf8 final
	{
	public:
		utf8() = delete;
		utf8(const utf8& encoding) = delete;
		utf8(utf8&& encoding) noexcept = delete;
		~utf8() = default;

	public:
		utf8& operator=(const utf8& encoding) = delete;
		utf8& operator=(utf8&& encoding) noexcept = delete;
		bool operator==(const utf8& encoding) = delete;
		bool operator!=(const utf8& encoding) = delete;

	public:
		static std::string encode_char(char32_t character);
		static std::string encode(const std::u32string_view& string);
		static char32_t decode_char(const std::string_view& string);
		static char32_t decode_char(char first, char second = 0, char third = 0, char fourth = 0);
		static std::u32string decode(const std::string_view& string);
		static std::size_t encoded_length(char32_t character);
		static std::size_t encoded_length(const std::u32string_view& string);
		static std::size_t decoded_length(const std::string_view& string);

	public:
		static constexpr const char* name = "UTF-8";
		static constexpr std::uint16_t codepage = 65001;
		static constexpr bool is_little_endian = false;
		static constexpr int max_encoded_length = 4;
		static constexpr std::uint8_t byte_order_mark[] = { 0xEF, 0xBB, 0xBF };
	};
}

#endif