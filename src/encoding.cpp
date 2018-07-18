#include <Dlink/encoding.hpp>

#include <Dlink/system.hpp>

#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace dlink::encoding
{
	encoding_type detect_encoding(std::istream& stream)
	{
		static const char zero[2]{ 0, 0 };

		char bom_buffer[4];
		stream.read(bom_buffer, 3);
		
		if (std::equal(bom_buffer, bom_buffer + 3, encoding::utf8::byte_order_mark))
		{
			return encoding_type::utf8;
		}
		else if (std::equal(bom_buffer, bom_buffer + 2, encoding::utf16::byte_order_mark))
		{
			stream.read(bom_buffer + 3, 1);

			if (std::equal(bom_buffer + 2, bom_buffer + 4, zero))
			{
				return encoding_type::utf32;
			}
			else
			{
				return encoding_type::utf16;
			}
		}
		else if (std::equal(bom_buffer, bom_buffer + 2, encoding::utf16be::byte_order_mark))
		{
			return encoding_type::utf16be;
		}
		else if (std::equal(bom_buffer, bom_buffer + 2, zero))
		{
			stream.read(bom_buffer + 3, 1);

			if (std::equal(bom_buffer + 2, bom_buffer + 4, encoding::utf16be::byte_order_mark))
			{
				return encoding_type::utf32be;
			}
		}

		return encoding_type::none;
	}
}

namespace dlink::encoding
{
	std::string utf8::encode_char(char32_t character)
	{
		if (is_invalid_unicode(character))
			throw std::invalid_argument("The argument 'character' isn't a valid Unicode character.");

		const std::size_t length = encoded_length(character);

		switch (length)
		{
		case 1:
			return { static_cast<char>(character) };

		case 2:
			return {
				static_cast<char>(0xC0 | (character >> 6)),
				static_cast<char>(0x80 | (character & 0x3F))
			};

		case 3:
			return {
				static_cast<char>(0xE0 | (character >> 12)),
				static_cast<char>(0x80 | ((character >> 6) & 0x3F)),
				static_cast<char>(0x80 | (character & 0x3F))
			};

		default:
			return {
				static_cast<char>(0xF0 | (character >> 18)),
				static_cast<char>(0x80 | ((character >> 12) & 0x3F)),
				static_cast<char>(0x80 | ((character >> 6) & 0x3F)),
				static_cast<char>(0x80 | (character & 0x3F))
			};
		}
	}
	std::string utf8::encode(const std::u32string_view& string)
	{
		if (string.empty())
			return "";

		std::string result;

		for (char32_t character : string)
		{
			result.append(encode_char(character));
		}

		return result;
	}
	char32_t utf8::decode_char(const std::string_view& string)
	{
		if (string.empty())
			return 0;
		if (string.size() > max_encoded_length)
			throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-8.");
		
		const std::size_t length = encoded_length(string[0]);

		switch (length)
		{
		case 1:
			return string[0];

		case 2:
			return ((string[0] & 0x1F) << 6) + (string[1] & 0x3F);

		case 3:
			return ((string[0] & 0x0F) << 12) + ((string[1] & 0x3F) << 6) + (string[2] & 0x3F);

		default:
			return ((string[0] & 0x07) << 18) + ((string[1] & 0x3F) << 12) + ((string[2] & 0x3F) << 6) + (string[3] & 0x3F);
		}
	}
	char32_t utf8::decode_char(char first, char second, char third, char fourth)
	{
		const unsigned char ufirst = first;
		const unsigned char usecond = second;
		const unsigned char uthird = third;
		const unsigned char ufourth = fourth;

		const std::size_t length = decoded_length(ufirst);

		switch (length)
		{
		case 1:
			return ufirst;

		case 2:
			return ((ufirst & 0x1F) << 6) + (usecond & 0x3F);

		case 3:
			return ((ufirst & 0x0F) << 12) + ((usecond & 0x3F) << 6) + (uthird & 0x3F);

		default:
			return ((ufirst & 0x07) << 18) + ((usecond & 0x3F) << 12) + ((uthird & 0x3F) << 6) + (ufourth & 0x3F);
		}
	}
	std::u32string utf8::decode(const std::string_view& string)
	{
		if (string.empty())
			return U"";

		using ssize_t = std::make_signed_t<std::size_t>;

		std::u32string result;

		for (std::size_t i = 0; i < string.size(); )
		{
			const char character = string[i];
			const std::size_t length = decoded_length(character);

			switch (length)
			{
			case 1:
			{
				result.push_back(decode_char(character));
				i += 1;

				break;
			}

			case 2:
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 2 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");

				result.push_back(decode_char(character, string[i + 1]));
				i += 2;

				break;
			}

			case 3:
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 3 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");

				result.push_back(decode_char(character, string[i + 1], string[i + 2]));
				i += 3;

				break;
			}

			default:
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 4 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");

				result.push_back(decode_char(character, string[i + 1], string[i + 2], string[i + 3]));
				i += 4;

				break;
			}
			}
		}

		return result;
	}
	std::size_t utf8::encoded_length(char32_t character)
	{
		if (is_invalid_unicode(character))
			throw std::invalid_argument("The argument 'character' isn't a valid Unicode character.");

		if (character < 0x80)
		{
			return 1;
		}
		else if (character < 0x800)
		{
			return 2;
		}
		else if (character < 0x10000)
		{
			return 3;
		}
		else
		{
			return 4;
		}
	}
	std::size_t utf8::encoded_length(const std::u32string_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;
		
		for (char32_t character : string)
		{
			result += encoded_length(character);
		}

		return result;
	}
	std::size_t utf8::decoded_length(char first)
	{
		const unsigned char ufirst = first;

		if (ufirst < 0x80)
		{
			return 1;
		}
		else if ((ufirst & 0xF0) == 0xF0)
		{
			return 4;
		}
		else if ((ufirst & 0xE0) == 0xE0)
		{
			return 3;
		}
		else if ((ufirst & 0xC0) == 0xC0)
		{
			return 2;
		}

		throw std::invalid_argument("The argument 'first' isn't a first valid byte encoded in UTF-8.");
	}
	std::size_t utf8::decoded_length(const std::string_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (std::size_t i = 0; i < string.size(); )
		{
			char character = string[i];
			std::size_t length = encoded_length(character);

			result += length;
			i += length;
		}

		return result;
	}
}

namespace dlink::encoding
{
	std::u16string utf16::encode_char(char32_t character)
	{
		if (is_invalid_unicode(character))
			throw std::invalid_argument("The argument 'character' isn't a valid Unicode character.");
		
		const std::size_t length = encoded_length(character);

		if (length == 1)
		{
			if (get_endian() == endian::big_endian)
			{
				std::swap(*(reinterpret_cast<std::uint8_t*>(&character) + 2),
						  *(reinterpret_cast<std::uint8_t*>(&character) + 3));
			}

			return { static_cast<char16_t>(character) };
		}
		else
		{
			char32_t temp = character - 0x10000;

			char16_t high_surrogate = static_cast<char16_t>((temp / 0x400) + 0xD800);
			char16_t low_surrogate = static_cast<char16_t>((temp % 0x400) + 0xDC00);

			if (get_endian() == endian::big_endian)
			{
				std::swap(*reinterpret_cast<std::uint8_t*>(&high_surrogate),
						 *(reinterpret_cast<std::uint8_t*>(&high_surrogate) + 1));
				std::swap(*reinterpret_cast<std::uint8_t*>(&low_surrogate),
						  *(reinterpret_cast<std::uint8_t*>(&low_surrogate) + 1));
			}

			return { high_surrogate, low_surrogate };
		}
	}
	std::u16string utf16::encode(const std::u32string_view& string)
	{
		if (string.empty())
			return u"";

		std::u16string result;

		for (char32_t character : string)
		{
			result.append(encode_char(character));
		}

		return result;
	}
	char32_t utf16::decode_char(const std::u16string_view& string)
	{
		if (string.empty())
			return 0;
		if (string.size() > max_encoded_length)
			throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-16.");

		char16_t first = string[0];
		char16_t second;

		if (get_endian() == endian::big_endian)
		{
			std::swap(*reinterpret_cast<std::uint8_t*>(&first),
					  *(reinterpret_cast<std::uint8_t*>(&first) + 1));
		}

		if (encoded_length(first) == 1)
		{
			return first;
		}
		else
		{
			if (string.size() == 1)
				throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-16.");

			second = string[1];

			if (get_endian() == endian::big_endian)
			{
				std::swap(*reinterpret_cast<std::uint8_t*>(&second),
						  *(reinterpret_cast<std::uint8_t*>(&second) + 1));
			}

			return (first << 16) + second;
		}
	}
	char32_t utf16::decode_char(char16_t first, char16_t second)
	{
		if (get_endian() == endian::big_endian)
		{
			std::swap(*reinterpret_cast<std::uint8_t*>(&first),
					  *(reinterpret_cast<std::uint8_t*>(&first) + 1));
		}

		if (encoded_length(first) == 1)
		{
			return first;
		}
		else
		{
			if (get_endian() == endian::big_endian)
			{
				std::swap(*reinterpret_cast<std::uint8_t*>(&second),
						  *(reinterpret_cast<std::uint8_t*>(&second) + 1));
			}

			return (first << 16) + second;
		}
	}
	std::u32string utf16::decode(const std::u16string_view& string)
	{
		if (string.empty())
			return U"";

		using ssize_t = std::make_signed_t<std::size_t>;

		std::u32string result;

		for (std::size_t i = 0; i < string.size(); )
		{
			char16_t character = string[i];
			std::size_t length = encoded_length(character);

			switch (length)
			{
			case 1:
			{
				result.push_back(decode_char(character));
				i += 1;

				break;
			}

			case 2:
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 2 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-16.");

				result.push_back(decode_char(character, string[i + 1]));
				i += 2;

				break;
			}
			}
		}

		return result;
	}
	std::size_t utf16::encoded_length(char32_t character)
	{
		if (is_invalid_unicode(character))
			throw std::invalid_argument("The argument 'character' isn't a valid Unicode character.");

		if (character < 0x10000)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
	std::size_t utf16::encoded_length(const std::u32string_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (char32_t character : string)
		{
			result += encoded_length(character);
		}

		return result;
	}
	std::size_t utf16::decoded_length(char16_t first)
	{
		if (first >= 0xD800 && first <= 0xDBFF)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
	std::size_t utf16::decoded_length(const std::u16string_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (std::size_t i = 0; i < string.size(); )
		{
			char16_t character = string[i];
			std::size_t length = decoded_length(character);

			result += length;
			i += length;
		}

		return result;
	}

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	std::wstring utf16::wencode_char(char32_t character)
	{
		std::wstring result;
		std::u16string temp = encode_char(character);

		result.insert(result.end(), temp.begin(), temp.end());
		return result;
	}
	std::wstring utf16::wencode(const std::u32string_view& string)
	{
		std::wstring result;
		std::u16string temp = encode(string);

		result.insert(result.end(), temp.begin(), temp.end());
		return result;
	}
	char32_t utf16::wdecode_char(const std::wstring_view& string)
	{
		switch (string.length())
		{
		case 1:
			return decode_char(static_cast<char16_t>(string[0]));

		case 2:
			return decode_char(static_cast<char16_t>(string[0]), static_cast<char16_t>(string[1]));

		default:
			throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-16.");
		}
	}
	char32_t utf16::wdecode_char(wchar_t first, wchar_t second)
	{
		return decode_char(static_cast<char16_t>(first), static_cast<char16_t>(second));
	}
	std::u32string utf16::wdecode(const std::wstring_view& string)
	{
		if (string.empty())
			return U"";

		std::u32string result;

		for (wchar_t character : string)
		{
			result.push_back(wdecode_char(character));
		}

		return result;
	}
	std::size_t utf16::wdecoded_length(wchar_t first)
	{
		return decoded_length(static_cast<char16_t>(first));
	}
	std::size_t utf16::wdecoded_length(const std::wstring_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (wchar_t character : string)
		{
			result += wdecoded_length(character);
		}

		return result;
	}
#endif
}

namespace dlink::encoding
{
	std::u16string utf16be::encode_char(char32_t character)
	{
		if (is_invalid_unicode(character))
			throw std::invalid_argument("The argument 'character' isn't a valid Unicode character.");

		std::size_t length = encoded_length(character);

		if (length == 1)
		{
			if (get_endian() == endian::little_endian)
			{
				std::swap(*reinterpret_cast<std::uint8_t*>(&character),
						  *(reinterpret_cast<std::uint8_t*>(&character) + 1));
			}

			return { static_cast<char16_t>(character) };
		}
		else
		{
			char32_t temp = character - 0x10000;

			char16_t high_surrogate = static_cast<char16_t>((temp / 0x400) + 0xD800);
			char16_t low_surrogate = static_cast<char16_t>((temp % 0x400) + 0xDC00);

			if (get_endian() == endian::little_endian)
			{
				std::swap(*reinterpret_cast<std::uint8_t*>(&high_surrogate),
						  *(reinterpret_cast<std::uint8_t*>(&high_surrogate) + 1));
				std::swap(*reinterpret_cast<std::uint8_t*>(&low_surrogate),
						  *(reinterpret_cast<std::uint8_t*>(&low_surrogate) + 1));
			}

			return { high_surrogate, low_surrogate };
		}
	}
	std::u16string utf16be::encode(const std::u32string_view& string)
	{
		if (string.empty())
			return u"";

		std::u16string result;

		for (char32_t character : string)
		{
			result.append(encode_char(character));
		}

		return result;
	}
	char32_t utf16be::decode_char(const std::u16string_view& string)
	{
		if (string.empty())
			return 0;
		if (string.size() > max_encoded_length)
			throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-16BE.");

		char16_t first = string[0];
		char16_t second;

		if (get_endian() == endian::little_endian)
		{
			std::swap(*reinterpret_cast<std::uint8_t*>(&first),
					  *(reinterpret_cast<std::uint8_t*>(&first) + 1));
		}

		if (encoded_length(first) == 1)
		{
			return first;
		}
		else
		{
			if (string.size() == 1)
				throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-16BE.");

			second = string[1];

			if (get_endian() == endian::little_endian)
			{
				std::swap(*reinterpret_cast<std::uint8_t*>(&second),
						  *(reinterpret_cast<std::uint8_t*>(&second) + 1));
			}

			return (first << 16) + second;
		}
	}
	char32_t utf16be::decode_char(char16_t first, char16_t second)
	{
		if (get_endian() == endian::little_endian)
		{
			std::swap(*reinterpret_cast<std::uint8_t*>(&first),
					  *(reinterpret_cast<std::uint8_t*>(&first) + 1));
		}

		if (encoded_length(first) == 1)
		{
			return first;
		}
		else
		{
			if (get_endian() == endian::little_endian)
			{
				std::swap(*reinterpret_cast<std::uint8_t*>(&second),
						  *(reinterpret_cast<std::uint8_t*>(&second) + 1));
			}

			return (first << 16) + second;
		}
	}
	std::u32string utf16be::decode(const std::u16string_view& string)
	{
		if (string.empty())
			return U"";

		using ssize_t = std::make_signed_t<std::size_t>;

		std::u32string result;

		for (std::size_t i = 0; i < string.size(); )
		{
			char16_t character = string[i];
			std::size_t length = encoded_length(character);

			switch (length)
			{
			case 1:
			{
				result.push_back(decode_char(character));
				i += 1;

				break;
			}

			case 2:
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 2 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-16BE.");

				result.push_back(decode_char(character, string[i + 1]));
				i += 2;

				break;
			}
			}
		}

		return result;
	}
	std::size_t utf16be::encoded_length(char32_t character)
	{
		if (is_invalid_unicode(character))
			throw std::invalid_argument("The argument 'character' isn't a valid Unicode character.");

		if (character < 0x10000)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
	std::size_t utf16be::encoded_length(const std::u32string_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (char32_t character : string)
		{
			result += encoded_length(character);
		}

		return result;
	}
	std::size_t utf16be::decoded_length(char16_t first)
	{
		if (first >= 0xD800 && first <= 0xDBFF)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
	std::size_t utf16be::decoded_length(const std::u16string_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (std::size_t i = 0; i < string.size(); )
		{
			char16_t character = string[i];
			std::size_t length = decoded_length(character);

			result += length;
			i += length;
		}

		return result;
	}

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	std::wstring utf16be::wencode_char(char32_t character)
	{
		std::wstring result;
		std::u16string temp = encode_char(character);

		result.insert(result.end(), temp.begin(), temp.end());
		return result;
	}
	std::wstring utf16be::wencode(const std::u32string_view& string)
	{
		std::wstring result;
		std::u16string temp = encode(string);

		result.insert(result.end(), temp.begin(), temp.end());
		return result;
	}
	char32_t utf16be::wdecode_char(const std::wstring_view& string)
	{
		switch (string.length())
		{
		case 1:
			return decode_char(static_cast<char16_t>(string[0]));

		case 2:
			return decode_char(static_cast<char16_t>(string[0]), static_cast<char16_t>(string[1]));

		default:
			throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-16BE.");
		}
	}
	char32_t utf16be::wdecode_char(wchar_t first, wchar_t second)
	{
		return decode_char(static_cast<char16_t>(first), static_cast<char16_t>(second));
	}
	std::u32string utf16be::wdecode(const std::wstring_view& string)
	{
		if (string.empty())
			return U"";

		std::u32string result;

		for (wchar_t character : string)
		{
			result.push_back(wdecode_char(character));
		}

		return result;
	}
	std::size_t utf16be::wdecoded_length(wchar_t first)
	{
		return decoded_length(static_cast<char16_t>(first));
	}
	std::size_t utf16be::wdecoded_length(const std::wstring_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (wchar_t character : string)
		{
			result += wdecoded_length(character);
		}

		return result;
	}
#endif
}