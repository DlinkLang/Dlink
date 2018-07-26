#ifndef DLINK_HEADER_LEXER_HPP
#define DLINK_HEADER_LEXER_HPP

#include <Dlink/token.hpp>

#include <map>
#include <string_view>

namespace dlink
{
	extern const std::map<std::string_view, token_type> keywords;
}

#endif