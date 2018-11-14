#include <Dlink/lexer.hpp>

#include <Dlink/encoding.hpp>
#include <Dlink/exception.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#ifdef DLINK_MULTITHREADING
#	include <Dlink/threading.hpp>
#endif

namespace dlink
{
#define MAP_KEYWORD(keyword) MAP_KEYWORD_INTERNAL(keyword, keyword_) 
#define MAP_KEYWORD_INTERNAL(keyword, dummy) std::pair<std::string_view, token_type>(#keyword , token_type:: dummy##keyword)
	const std::map<std::string_view, token_type> keywords =
	{
		MAP_KEYWORD(auto),
		MAP_KEYWORD(void),
		MAP_KEYWORD(bool),
		MAP_KEYWORD(char),
		MAP_KEYWORD(char16),
		MAP_KEYWORD(char32),

		MAP_KEYWORD(i8),
		MAP_KEYWORD(i16),
		MAP_KEYWORD(i32),
		MAP_KEYWORD(i64),
		MAP_KEYWORD(u8),
		MAP_KEYWORD(u16),
		MAP_KEYWORD(u32),
		MAP_KEYWORD(u64),

		MAP_KEYWORD(let),
		MAP_KEYWORD(immut),
		MAP_KEYWORD(mut),
		MAP_KEYWORD(const),

		MAP_KEYWORD(func),
		MAP_KEYWORD(class),
		MAP_KEYWORD(union),
		MAP_KEYWORD(module),
		MAP_KEYWORD(domain),
		MAP_KEYWORD(inline),
		MAP_KEYWORD(enum),
		MAP_KEYWORD(public),
		MAP_KEYWORD(internal),
		MAP_KEYWORD(protected),
		MAP_KEYWORD(private),
		MAP_KEYWORD(use),
		MAP_KEYWORD(as),
		MAP_KEYWORD(default),
		MAP_KEYWORD(macro),
		MAP_KEYWORD(panic),

		MAP_KEYWORD(for),
		MAP_KEYWORD(do),
		MAP_KEYWORD(while),
		MAP_KEYWORD(match),
		MAP_KEYWORD(if),
		MAP_KEYWORD(else),
		MAP_KEYWORD(goto),
		MAP_KEYWORD(break),
		MAP_KEYWORD(continue),
		MAP_KEYWORD(return),

		MAP_KEYWORD(extern),
		MAP_KEYWORD(template),
		MAP_KEYWORD(type),
		MAP_KEYWORD(concept),
		MAP_KEYWORD(unsafe),

		MAP_KEYWORD(virtual),
		MAP_KEYWORD(abstract),
		MAP_KEYWORD(open),
		MAP_KEYWORD(this),
		MAP_KEYWORD(super),
		MAP_KEYWORD(static),

		MAP_KEYWORD(async),
		MAP_KEYWORD(await),

		MAP_KEYWORD(bit),
		MAP_KEYWORD(new),
		MAP_KEYWORD(delete),
		MAP_KEYWORD(nullptr),

		MAP_KEYWORD(static_cast),
		MAP_KEYWORD(dynamic_cast),
		MAP_KEYWORD(const_cast),
		MAP_KEYWORD(reinterpret_cast),
		MAP_KEYWORD(is),
		MAP_KEYWORD(typeid),

		MAP_KEYWORD(static_assert),

		MAP_KEYWORD(true),
		MAP_KEYWORD(false),
	};
#undef MAP_KEYWORD
#undef MAP_KEYWORD_INTERNAL
}

namespace dlink
{
	namespace
	{
		bool getline(std::istream& stream, const char* org_string, std::string_view& output)
		{
			if (stream.eof())
			{
				output = "";
				return false;
			}

			std::size_t length = 0;
			const std::size_t pos = static_cast<std::size_t>(stream.tellg());
			
			while (!is_eol(stream))
			{
				++length;
				stream.seekg(1, std::ios::cur);
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

	bool lexer::lex(compiler_metadata& metadata, std::vector<source>& sources)
	{
#ifdef DLINK_MULTITHREADING
		auto lex_multithread = [&](std::size_t begin, std::size_t end) -> bool
		{
			bool result = true;

			for (std::size_t i = begin; i < end; ++i)
			{
				result = result && lex_source(sources[i], metadata);
			}

			return result;
		};

		return parallel(lex_multithread, get_threading_info(metadata));
#else
		return lex_singlethread(metadata, sources);
#endif
	}
	bool lexer::lex_singlethread(compiler_metadata& metadata, std::vector<source>& sources)
	{
		bool result = true;

		for (source& src : sources)
		{
			result = result && lex_source(src, metadata);
		}

		return result;
	}
	bool lexer::lex_source(source& source, compiler_metadata& metadata)
	{
		if (source.state() != source_state::decoded)
			throw invalid_state("The state of the argument 'source' must be 'dlink::source_state::decoded' when 'static bool dlink::lexer::lex_source(dlink::source&, dlink::compiler_metadata&)' method is called.");
		
		std::vector<token> tokens;
		if (!lex_preprocess_(source, metadata, tokens)) return false;

		bool ok = true;

		for (std::size_t i = 0; i < tokens.size(); ++i)
		{
			token& cur_token = tokens[i];
			const token_type cur_token_type = cur_token.type();

			if (cur_token_type == token_type::none_hm)
			{
#define make_internal_lexing_data() (internal_lexing_data_{ source, metadata, tokens, cur_token })

				const char first_c = cur_token.data()[0];

				if (std::isdigit(first_c))
				{
					ok = ok && lex_number_(make_internal_lexing_data());
				}

#undef make_internal_lexing_data
			}
		}

		if (ok)
		{
			source.tokens(std::move(tokens));
		}

		return ok;
	}

	bool lexer::lex_preprocess_(source& source, compiler_metadata& metadata, std::vector<token>& tokens)
	{
		using memstream = boost::iostreams::stream<boost::iostreams::basic_array_source<char>>;

		memstream stream(const_cast<char*>(source.codes().c_str()), source.codes().length());
		std::size_t line = 0;
		std::string_view current_line;
		bool ok = true;

		bool multiline_comment = false;
		std::size_t multiline_comment_line = 0, multiline_comment_col = 0;
		std::string_view multiline_comment_line_data;

		while (getline(stream, source.codes().c_str(), current_line))
		{
			++line;

			const std::size_t length = current_line.size();
			memstream line_stream(current_line.data(), length);

			std::size_t hm_length = 0;
			char next_c;
			
			bool string = false;
			bool character = false;
			std::size_t string_or_character_line = 0, string_or_character_col = 0;

			while (!line_stream.eof())
			{
				const std::size_t offset = static_cast<std::size_t>(line_stream.tellg());

				if (is_whitespace(line_stream, next_c))
				{
					if (hm_length)
					{
						const std::size_t hm_offset = offset - hm_length;
						tokens.emplace_back(current_line.substr(hm_offset, hm_length), token_type::none_hm, line, hm_offset, current_line);
						hm_length = 0;
					}
				}
				else if (is_special_character(next_c))
				{					
					if (hm_length && !string && !character)
					{
						const std::size_t hm_offset = offset - hm_length;
						tokens.emplace_back(current_line.substr(hm_offset, hm_length), token_type::none_hm, line, hm_offset, current_line);
						hm_length = 0;
					}

					if (!multiline_comment && next_c == '/')
					{
						line_stream.get(next_c);
						if (line_stream.good() && next_c == '*')
						{
							multiline_comment = true;
							multiline_comment_line = line;
							multiline_comment_col = offset + 1;
							multiline_comment_line_data = current_line;
						}
						else if (next_c == '/')
						{
							break;
						}
						else goto add;
					}
					else if (multiline_comment && next_c == '*')
					{
						line_stream.get(next_c);
						if (line_stream.good() && next_c == '/')
						{
							multiline_comment = false;
						}
						else goto add;
					}
					else if (!string && !character)
					{
						if (next_c == '"')
						{
							hm_length = 1;
							string = true;
							string_or_character_line = line;
							string_or_character_col = offset + 1;
						}
						else if (next_c == '\'')
						{
							hm_length = 1;
							character = true;
							string_or_character_line = line;
							string_or_character_col = offset + 1;
						}
						else goto add;
					}
					else if (string && !character)
					{
						if (next_c == '"')
						{
							tokens.emplace_back(current_line.substr(offset - hm_length, hm_length + 1), token_type::string, line, offset - hm_length, current_line);
							hm_length = 0;
							string = false;
						}
						else if (next_c == '\\')
						{
							hm_length += 2;
						}
						else goto add;
					}
					else if (character && !string)
					{
						if (next_c == '\'')
						{
							tokens.emplace_back(current_line.substr(offset - hm_length, hm_length + 1), token_type::character, line, offset - hm_length, current_line);
							hm_length = 0;
							character = false;
						}
						else if (next_c == '\\')
						{
							hm_length += 2;
						}
						else goto add;
					}
					else
					{
					add:
						if ((string && next_c == '\'') || (character && next_c == '"'))
						{
							++hm_length;
						}
						else if (!is_valid_special_character(next_c))
						{
							using namespace std::string_literals;

							metadata.messages().push_back(std::make_shared<error_message>(
								2006, "'"s + next_c + "' is an invalid token.",
								generate_line_col(source.path(), line, offset + 1),
								generate_source(current_line, line, offset + 1, 1)
								));
							ok = false;
						}
						else
						{
							token& added_token = tokens.emplace_back(current_line.substr(offset, 1), to_token_type(next_c), line, offset, current_line);
							if (!is_single_special_character(next_c))
							{
								static constexpr int max_depth = 3;
								token_type old_type = added_token.type();

								for (int i = 1; i < max_depth; ++i)
								{
									line_stream.read(&next_c, 1);
									if (line_stream.good())
									{
										if (is_valid_special_character(next_c))
										{
											added_token.type(complex_token_type(added_token.type(), next_c));

											if (const token_type new_type = added_token.type(); new_type != old_type)
											{
												old_type = new_type;
												added_token.data(current_line.substr(offset, i + 1));
											}
											else
											{
												line_stream.seekg(-1, std::ios::cur);
												break;
											}
										}
										else
										{
											line_stream.seekg(-1, std::ios::cur);
											break;
										}
									}
									else break;
								}
							}
						}
					}
				}
				else if (!multiline_comment)
				{
					++hm_length;
				}
			}

			if (character)
			{
				using namespace std::string_literals;

				metadata.messages().push_back(std::make_shared<error_message>(
					2008, "Unexpected EOL found in character literal.",
					generate_line_col(source.path(), string_or_character_line, string_or_character_col),
					generate_source(current_line, string_or_character_line, string_or_character_col, 1)
					));
				ok = false;
			}
			else if (string)
			{
				using namespace std::string_literals;

				metadata.messages().push_back(std::make_shared<error_message>(
					2009, "Unexpected EOL found in string literal.",
					generate_line_col(source.path(), string_or_character_line, string_or_character_col),
					generate_source(current_line, string_or_character_line, string_or_character_col, 1)
					));
				ok = false;
			}
			else if (hm_length)
			{
				const std::size_t hm_offset = length - hm_length;
				tokens.emplace_back(current_line.substr(hm_offset, hm_length), token_type::none_hm, line, hm_offset, current_line);
			}
		}

		if (multiline_comment)
		{
			using namespace std::string_literals;

			metadata.messages().push_back(std::make_shared<error_message>(
				2007, "Unexpected EOF found in comment.",
				generate_line_col(source.path(), multiline_comment_line, multiline_comment_col),
				generate_source(multiline_comment_line_data, multiline_comment_line, multiline_comment_col, 2)
				));
			ok = false;
		}
		return ok;
	}
	bool lexer::lex_number_(internal_lexing_data_ data)
	{
		using namespace std::string_literals;

		const std::string_view& token_data = data.token.data();
		const std::size_t token_line = data.token.line();
		const std::size_t token_col = data.token.col() + 1;

		if (token_data[0] == '0')
		{
			if (token_data.size() == 1)
			{
				data.token.type(token_type::integer_dec);
				return true;
			}

			switch (token_data[1])
			{
			case 'B':
			case 'b':
				return lex_number_with_base_(data, 2);

			case 'X':
			case 'x':
				return lex_number_with_base_(data, 16);
			}

			data.token.type(token_type::integer_oct);

			for (std::size_t i = 1; i < token_data.size(); ++i)
			{
				const char c = token_data[i];

				const bool is_underbar = c == '_';
				const bool is_valid_digit = (c >= '0' && c <= '7') || is_underbar;

				if (!is_valid_digit)
				{
					if (std::isdigit(c))
					{
						data.metadata.messages().push_back(std::make_shared<error_message>(
							2001, "Invalid digit '"s + c + "' in octal literal.",
							generate_line_col(data.source.path(), token_line, i + 1),
							generate_source(data.token.line_data(), token_line, i + 1, 1)
							));
						return false;
					}
					else
					{
						data.token.postfix_literal(token_data.substr(i));
						data.token.data(token_data.substr(0, i));
						return true;
					}
				}
			}
		}
		else
		{
			data.token.type(token_type::integer_dec);

			for (std::size_t i = 1; i < token_data.size(); ++i)
			{
				const char c = token_data[i];

				const bool is_underbar = c == '_';
				const bool is_valid_digit = std::isdigit(c) || is_underbar;

				if (!is_valid_digit)
				{
					data.token.postfix_literal(token_data.substr(i));
					data.token.data(token_data.substr(0, i));
					return true;
				}
			}
		}

		return true;
	}
	bool lexer::lex_number_with_base_(internal_lexing_data_ data, int base)
	{
		using namespace std::string_literals;

		data.token.type(base == 2 ? token_type::integer_bin : token_type::integer_hex);

		const auto is_digit = [base](char character)
		{
			if (base == 2)
			{
				return character == '0' || character == '1';
			}
			else // base == 16
			{
				return std::isdigit(character) || (character >= 'a' && character <= 'f') || (character >= 'A' && character <= 'F');
			}
		};
		const auto error_id_invalid_digit = [base]()
		{
			if (base == 2)
			{
				return 2000;
			}
			else // base == 16
			{
				return 2003;
			}
		};
		const auto error_id_invalid_format = [base]()
		{
			if (base == 2)
			{
				return 2004;
			}
			else // base == 16
			{
				return 2005;
			}
		};
		const auto base_string = [base]()
		{
			if (base == 2)
			{
				return "binary";
			}
			else // base == 16
			{
				return "hexadecimal";
			}
		};

		const std::size_t token_line = data.token.line();
		const std::size_t token_col = data.token.col() + 1;

		if (data.token.data().size() == 2)
		{
			data.metadata.messages().push_back(std::make_shared<error_message>(
				error_id_invalid_format(), "Invalid "s + base_string() + " literal.",
				generate_line_col(data.source.path(), token_line, token_col),
				generate_source(data.token.line_data(), token_line, token_col, 2)
				));
			return false;
		}

		const std::string_view token_data = data.token.data().substr(2);
		
		for (std::size_t i = 0; i < token_data.size(); ++i)
		{
			const char c = token_data[i];

			const bool is_underbar = c == '_';
			const bool is_valid_digit = is_digit(c) || is_underbar;

			if (!is_valid_digit)
			{
				if (base == 2 && std::isdigit(c))
				{
					data.metadata.messages().push_back(std::make_shared<error_message>(
						error_id_invalid_digit(), "Invalid digit '"s + c + "' in " + base_string() + " literal.",
						generate_line_col(data.source.path(), token_line, i + 3),
						generate_source(data.token.line_data(), token_line, i + 3, 1)
						));
					return false;
				}
				else
				{
					data.token.postfix_literal(token_data.substr(i));
					data.token.data(token_data.substr(0, i));
					return true;
				}
			}
		}

		return true;
	}
}