#ifndef DLINK_HEADER_LEXER_HPP
#define DLINK_HEADER_LEXER_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/source.hpp>
#include <Dlink/token.hpp>

#include <istream>
#include <map>
#include <string_view>
#include <vector>

namespace dlink
{
	extern const std::map<std::string_view, token_type> keywords;

	class lexer final
	{
	private:
		struct internal_lexing_data_
		{
			dlink::source& source;
			compiler_metadata& metadata;
			std::vector<token>& tokens;
			token& token;
			std::size_t& token_index;
		};

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

	private:
		static bool lex_preprocess_(source& source, compiler_metadata& metadata, std::vector<token>& tokens);
		static bool lex_number_(internal_lexing_data_ data, bool cannot_sn = false);
		static bool lex_number_with_base_(internal_lexing_data_ data, int base);
	};
}

#endif