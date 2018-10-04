#include <Dlink/utility.hpp>

#include <Dlink/vector.hpp>

namespace dlink
{
	const std::string& generate_indent(std::size_t depth)
	{
		static vector<std::string> indents{ "" };

		if (indents.size() <= depth)
		{
			indents.push_back(generate_indent(depth - 1) + "    ");
		}

		return indents[depth];
	}
}