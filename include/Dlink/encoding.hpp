#ifndef DLINK_HEADER_ENCODING_HPP
#define DLINK_HEADER_ENCODING_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

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
		static std::size_t decoded_length(char first);
		static std::size_t decoded_length(const std::string_view& string);

	public:
		static constexpr const char* name = "UTF-8";
		static constexpr std::uint16_t codepage = 65001;
		static constexpr bool is_little_endian = false;
		static constexpr std::int32_t max_encoded_length = 4;
		static constexpr std::uint8_t byte_order_mark[] = { 0xEF, 0xBB, 0xBF };
	};

	class utf16 final
	{
	public:
		utf16() = delete;
		utf16(const utf16& encoding) = delete;
		utf16(utf16&& encoding) noexcept = delete;
		~utf16() = default;

	public:
		utf16& operator=(const utf16& encoding) = delete;
		utf16& operator=(utf16&& encoding) noexcept = delete;
		bool operator==(const utf16& encoding) = delete;
		bool operator!=(const utf16& encoding) = delete;

	public:
		static std::u16string encode_char(char32_t character);
		static std::u16string encode(const std::u32string_view& string);
		static char32_t decode_char(const std::u16string_view& string);
		static char32_t decode_char(char16_t first, char16_t second = 0);
		static std::u32string decode(const std::u16string_view& string);
		static std::size_t encoded_length(char32_t character);
		static std::size_t encoded_length(const std::u32string_view& string);
		static std::size_t decoded_length(char16_t first);
		static std::size_t decoded_length(const std::u16string_view& string);

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
		static std::wstring wencode_char(char32_t character);
		static std::wstring wencode(const std::u32string_view& string);
		static char32_t wdecode_char(const std::wstring_view& string);
		static char32_t wdecode_char(wchar_t first, wchar_t second = 0);
		static std::u32string wdecode(const std::wstring_view& string);
		static std::size_t wdecoded_length(wchar_t first);
		static std::size_t wdecoded_length(const std::wstring_view& string);
#endif

	public:
		static constexpr const char* name = "UTF-16";
		static constexpr std::uint16_t codepage = 1200;
		static constexpr bool is_little_endian = true;
		static constexpr std::int32_t max_encoded_length = 2;
		static constexpr std::uint8_t byte_order_mark[] = { 0xFF, 0xFE };
	};

	class utf16be final
	{
	public:
		utf16be() = delete;
		utf16be(const utf16be& encoding) = delete;
		utf16be(utf16be&& encoding) noexcept = delete;
		~utf16be() = default;

	public:
		utf16be& operator=(const utf16be& encoding) = delete;
		utf16be& operator=(utf16be&& encoding) noexcept = delete;
		bool operator==(const utf16be& encoding) = delete;
		bool operator!=(const utf16be& encoding) = delete;

	public:
		static std::u16string encode_char(char32_t character);
		static std::u16string encode(const std::u32string_view& string);
		static char32_t decode_char(const std::u16string_view& string);
		static char32_t decode_char(char16_t first, char16_t second = 0);
		static std::u32string decode(const std::u16string_view& string);
		static std::size_t encoded_length(char32_t character);
		static std::size_t encoded_length(const std::u32string_view& string);
		static std::size_t decoded_length(char16_t first);
		static std::size_t decoded_length(const std::u16string_view& string);

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
		static std::wstring wencode_char(char32_t character);
		static std::wstring wencode(const std::u32string_view& string);
		static char32_t wdecode_char(const std::wstring_view& string);
		static char32_t wdecode_char(wchar_t first, wchar_t second = 0);
		static std::u32string wdecode(const std::wstring_view& string);
		static std::size_t wdecoded_length(wchar_t first);
		static std::size_t wdecoded_length(const std::wstring_view& string);
#endif

	public:
		static constexpr const char* name = "UTF-16BE";
		static constexpr std::uint16_t codepage = 1201;
		static constexpr bool is_little_endian = false;
		static constexpr std::int32_t max_encoded_length = 2;
		static constexpr std::uint8_t byte_order_mark[] = { 0xFE, 0xFF };
	};
}

#endif