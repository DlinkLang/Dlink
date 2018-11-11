#include <Dlink/compilation_pipeline.hpp>

#include <utility>

int main(int argc, char** argv)
{
	dlink::compiler_options options;

	if (!dlink::parse_command_line(argc, argv, options))
	{
		return 0;
	}

	dlink::compilation_pipeline pipeline(std::move(options));
	
	pipeline.compile_until_lexing();
	pipeline.dump_messages();

	return 0;
}

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>

static_assert(sizeof(char) == sizeof(std::int8_t));
static_assert(sizeof(std::size_t) >= sizeof(std::int32_t));
static_assert(sizeof(std::fpos_t) >= sizeof(std::size_t));
static_assert(sizeof(std::int32_t) >= sizeof(int));
static_assert(CHAR_BIT == 8);