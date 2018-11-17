#ifndef DLINK_HEADER_UTILITY_HPP
#define DLINK_HEADER_UTILITY_HPP

#include <istream>
#include <string_view>
#include <vector>

namespace dlink
{
	bool getline(std::istream& stream, const char* org_string, std::string_view& output);
}

#endif