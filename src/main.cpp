#include <Dlink/compiler_options.hpp>

int main(int argc, char** argv)
{
	dlink::compiler_options options;

	if (!dlink::parse_command_line(argc, argv, options))
	{
		return 0;
	}

	return 0;
}