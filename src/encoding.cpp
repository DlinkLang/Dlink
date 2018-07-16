#include <Dlink/encoding.hpp>

#include <stdexcept>
#include <type_traits>

namespace dlink::encoding
{
	std::string utf8::encode_char(char32_t character)
	{
		if (is_invalid_unicode(character))
			throw std::invalid_argument("The argument 'character' isn't a valid Unicode character.");

		if (character < 0x80)
		{
			return { static_cast<char>(character) };
		}
		else if (character < 0x800)
		{
			return {
				static_cast<char>(0xC0 | (character >> 6)),
				static_cast<char>(0x80 | (character & 0x3F))
			};
		}
		else if (character < 0x10000)
		{
			return {
				static_cast<char>(0xE0 | (character >> 12)),
				static_cast<char>(0x80 | ((character >> 6) & 0x3F)),
				static_cast<char>(0x80 | (character & 0x3F))
			};
		}
		else
		{
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
		
		if (static_cast<std::uint8_t>(string[0]) < 0x80)
		{
			return string[0];
		}
		else if ((static_cast<std::uint8_t>(string[0]) & 0xF0) == 0xF0)
		{
			return ((string[0] & 0x07) << 18) + ((string[1] & 0x3F) << 12) + ((string[2] & 0x3F) << 6) + (string[3] & 0x3F);
		}
		else if ((static_cast<std::uint8_t>(string[0]) & 0xE0) == 0xE0)
		{
			return ((string[0] & 0x0F) << 12) + ((string[1] & 0x3F) << 6) + (string[2] & 0x3F);
		}
		else if ((static_cast<std::uint8_t>(string[0]) & 0xC0) == 0xC0)
		{
			return ((string[0] & 0x1F) << 6) + (string[1] & 0x3F);
		}

		throw std::invalid_argument("The argument 'string' isn't a valid Unicode character encoded in UTF-8.");
	}
	char32_t utf8::decode_char(char first, char second, char third, char fourth)
	{
		unsigned char first = first;
		unsigned char second = second;
		unsigned char third = third;
		unsigned char fourth = fourth;

		if (first < 0x80)
		{
			return first;
		}
		else if ((first & 0xF0) == 0xF0)
		{
			return ((first & 0x07) << 18) + ((second & 0x3F) << 12) + ((third & 0x3F) << 6) + (fourth & 0x3F);
		}
		else if ((first & 0xE0) == 0xE0)
		{
			return ((first & 0x0F) << 12) + ((second & 0x3F) << 6) + (third & 0x3F);
		}
		else if ((first & 0xC0) == 0xC0)
		{
			return ((first & 0x1F) << 6) + (second & 0x3F);
		}

		throw std::invalid_argument("The arguments aren't vaild bytes encoded in UTF-8.");
	}
	std::u32string utf8::decode(const std::string_view& string)
	{
		if (string.empty())
			return U"";

		using ssize_t = std::make_signed_t<std::size_t>;

		std::u32string result;

		for (std::size_t i = 0; i < string.size(); )
		{
			unsigned char character = string[i];

			if (character < 0x80)
			{
				result.push_back(decode_char(character));
				i += 1;
			}
			else if ((character & 0xF0) == 0xF0)
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 4 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");

				result.push_back(decode_char(character, string[i + 1], string[i + 2], string[i + 3]));
				i += 4;
			}
			else if ((character & 0xE0) == 0xE0)
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 3 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");

				result.push_back(decode_char(character, string[i + 1], string[i + 2]));
				i += 3;
			}
			else if ((character & 0xC0) == 0xC0)
			{
				if (static_cast<ssize_t>(string.size()) - static_cast<ssize_t>(i) - 2 < 0)
					throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");

				result.push_back(decode_char(character, string[i + 1]));
				i += 2;
			}

			throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");
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
	std::size_t utf8::decoded_length(const std::string_view& string)
	{
		if (string.empty())
			return 0;

		std::size_t result = 0;

		for (std::size_t i = 0; i < string.size(); )
		{
			unsigned char character = string[i];

			if (character < 0x80)
			{
				result += 1;
				i += 1;
			}
			else if ((character & 0xF0) == 0xF0)
			{
				result += 4;
				i += 4;
			}
			else if ((character & 0xE0) == 0xE0)
			{
				result += 3;
				i += 3;
			}
			else if ((character & 0xC0) == 0xC0)
			{
				result += 2;
				i += 2;
			}

			throw std::invalid_argument("The argument 'string' isn't a valid string encoded in UTF-8.");
		}

		return result;
	}
}