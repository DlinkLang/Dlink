#include <Dlink/utility.hpp>

#include <Dlink/encoding.hpp>

#include <cstddef>

namespace dlink
{
	bool getline(std::istream& stream, const char* org_string, std::string_view& output)
	{
		if (stream.eof())
		{
			output = "";
			return false;
		}

		std::size_t length = 0;
		int char_size;
		const std::size_t pos = static_cast<std::size_t>(stream.tellg());

		while (!is_eol(stream, char_size))
		{
			length += char_size;
			stream.seekg(char_size, std::ios::cur);
		}

		if (length == 0)
		{
			output = "";
			return false;
		}
		else
		{
			output = std::string_view(org_string + pos, length);
			return true;
		}
	}
}