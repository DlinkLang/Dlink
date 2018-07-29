#ifndef DLINK_HEADER_LEXER_HPP
#define DLINK_HEADER_LEXER_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/source.hpp>
#include <Dlink/token.hpp>

#include <map>
#include <string_view>

namespace dlink
{
	extern const std::map<std::string_view, token_type> keywords;

	class lexer final
	{
	public:
		lexer() = delete;
		lexer(const lexer& lexer) = delete;
		lexer(lexer&& lexer) noexcept = delete;
		~lexer() = default;

	public:
		lexer& operator=(const lexer& lexer) = delete;
		lexer& operator=(lexer&& lexer) noexcept = delete;
		bool operator==(const lexer& lexer) const = delete;
		bool operator!=(const lexer& lexer) const = delete;

	public:
		static bool lex(compiler_metadata& metadata, std::vector<source>& sources);
		static bool lex_singlethread(compiler_metadata& metadata, std::vector<source>& sources);
		static bool lex_source(source& source, compiler_metadata& metadata);
	};
}

#endif