#include <Dlink/compiler_metadata.hpp>
#include <Dlink/compiler_options.hpp>
#include <Dlink/decoder.hpp>
#include <Dlink/lexer.hpp>
#include <Dlink/message.hpp>

#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
	dlink::compiler_options options;

	if (!dlink::parse_command_line(argc, argv, options))
	{
		return 0;
	}

	dlink::compiler_metadata metadata(options);
	
	std::vector<dlink::source> results;
	dlink::decoder::decode(metadata, results);
	dlink::lexer::lex_singlethread(metadata, results);

	for (auto a : metadata.messages())
	{
		std::cout << dlink::to_string(a) << "\n\n";
	}

	return 0;
}

#include <climits>
#include <cstddef>
#include <cstdint>

static_assert(sizeof(char) == sizeof(std::int8_t));
static_assert(sizeof(std::size_t) >= sizeof(std::int32_t));
static_assert(sizeof(std::fpos_t) >= sizeof(std::size_t));
static_assert(CHAR_BIT == 8);