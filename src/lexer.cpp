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
		lex_preprocess_(source, tokens);

		/*using namespace std::string_literals;
		using memorystream = boost::iostreams::stream<boost::iostreams::basic_array_source<char>>;
		
		std::size_t line = 0;

		memorystream stream(
			const_cast<char*>(source.codes().c_str()), source.codes().length()
		);
		std::string_view current_line;

		dlink::tokens tokens;

		bool multiline_comment = false;
		bool ok = true;

#define make_internal_lexing_data() (internal_lexing_data_{ line_stream, current_line, length, line, c, source, metadata, tokens })

		while (getline(stream, source.codes().c_str(), current_line))
		{
			++line;
			memorystream line_stream(current_line.data(), current_line.length());

			const std::size_t length = current_line.size();

			while (!line_stream.eof())
			{
				const char c = static_cast<char>(line_stream.get());
				const std::size_t i = static_cast<std::size_t>(line_stream.tellg()) - 1;

				if (std::isdigit(c))
				{
					ok = ok && lex_number_(make_internal_lexing_data());
				}
			}
		}

#undef make_internal_lexing_data*/

		return false;

		/*if (ok)
		{
			source.tokens(std::move(tokens));
			return true;
		}
		else
		{
			return false;
		}*/
	}

	void lexer::lex_preprocess_(source& source, std::vector<token>& tokens)
	{
		using memstream = boost::iostreams::stream<boost::iostreams::basic_array_source<char>>;

		memstream stream(const_cast<char*>(source.codes().c_str()), source.codes().length());
		std::size_t line = 0;
		std::string_view current_line;

		bool multiline_comment = false;

		while (getline(stream, source.codes().c_str(), current_line))
		{
			++line;

			const std::size_t length = current_line.size();
			memstream line_stream(current_line.data(), length);

			std::size_t hm_length = 0;
			char next_c;

			while (!line_stream.eof())
			{
				const std::size_t offset = static_cast<std::size_t>(line_stream.tellg());

				if (is_whitespace(line_stream, next_c))
				{
					if (hm_length)
					{
						const std::size_t hm_offset = offset - hm_length;
						tokens.emplace_back(current_line.substr(hm_offset, hm_length), token_type::none_hm, line, hm_offset);
						hm_length = 0;
					}
				}
				else if (is_special_character(next_c))
				{
					if (hm_length)
					{
						const std::size_t hm_offset = offset - hm_length;
						tokens.emplace_back(current_line.substr(hm_offset, hm_length), token_type::none_hm, line, hm_offset);
						hm_length = 0;
					}

					if (!multiline_comment && next_c == '/')
					{
						line_stream.get(next_c);
						if (line_stream.good() && next_c == '*')
						{
							multiline_comment = true;
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
					else
					{
					add:
						tokens.emplace_back(current_line.substr(offset, 1), token_type::none_sc, line, offset);
					}
				}
				else if (!multiline_comment)
				{
					++hm_length;
				}
			}

			if (hm_length)
			{
				const std::size_t hm_offset = length - hm_length;
				tokens.emplace_back(current_line.substr(hm_offset, hm_length), token_type::none_hm, line, hm_offset);
			}
		}
	}
	bool lexer::lex_number_(internal_lexing_data_ data)
	{
		using namespace std::string_literals;

		const std::size_t c_pos = static_cast<std::size_t>(data.line_stream.tellg()) - 1;
		std::size_t length = 1;
		std::size_t post_literal_pos = 0;
		std::size_t post_literal_length = 0;

		dlink::token_type token_type = dlink::token_type::none;
		std::string_view prefix;

		bool error = false;

		if (data.c == '0')
		{
			char next_c;
			const bool ws = is_whitespace(data.line_stream, next_c);

			if (ws)
			{
				token_type = dlink::token_type::integer_dec;
				goto exit;
			}

			switch (next_c)
			{
			case 'B':
			case 'b':
				return lex_number_with_base_(data, 2);

			case 'X':
			case 'x':
				return lex_number_with_base_(data, 16);
			}

			token_type = dlink::token_type::integer_oct;

			do
			{
				const bool is_valid_digit = (next_c >= '0' && next_c <= '7') || next_c == '_';

				if (!post_literal_pos && is_valid_digit)
				{
					++length;
				}
				else if (!post_literal_pos && !is_valid_digit)
				{
					if (std::isdigit(next_c))
					{
						error = true;

						const std::size_t pos = static_cast<std::size_t>(data.line_stream.tellg());
						data.metadata.messages().push_back(std::make_shared<error_message>(
							2001, "Invalid digit '"s + next_c + "' in octal literal.",
							generate_line_col(data.source.path(), data.line_line, pos),
							generate_source(data.line, data.line_line, pos, 1)
							));
					}
					else
					{
						token_type = dlink::token_type::integer_dec;
						post_literal_pos = length;
						post_literal_length++;
					}
				}
				else
				{
					if (!post_literal_pos)
					{
						post_literal_pos = length;
					}
					post_literal_length++;
				}
			} while (!is_whitespace(data.line_stream, next_c));

			goto exit;
		}
		
		char next_c;

		while (!is_whitespace(data.line_stream, next_c))
		{
			const bool is_valid_digit = std::isdigit(next_c) || next_c == '_';

			if (!post_literal_pos && is_valid_digit)
			{
				++length;
			}
			else
			{
				if (!post_literal_pos)
				{
					post_literal_pos = length;
				}
				post_literal_length++;
			}
		}

		token_type = dlink::token_type::integer_dec;

	exit:
		if (!error)
		{
			data.tokens.emplace_back(data.line.substr(c_pos, length), token_type, data.line_line, c_pos, prefix,
				data.line.substr(post_literal_pos, post_literal_length));
			return true;
		}
		else
		{
			return false;
		}
	}
	bool lexer::lex_number_with_base_(internal_lexing_data_ data, int base)
	{
		using namespace std::string_literals;

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

		const std::size_t c_pos = static_cast<std::size_t>(data.line_stream.tellg()) - 2;
		std::size_t length = 2;
		std::size_t post_literal_pos = 0;
		std::size_t post_literal_length = 0;

		dlink::token_type token_type = base == 2 ? dlink::token_type::integer_bin : dlink::token_type::integer_hex;
		std::string_view prefix;

		char next_c;
		bool error = false;

		while (!is_whitespace(data.line_stream, next_c))
		{
			const bool is_valid_digit = is_digit(next_c) || next_c == '_';

			if (!post_literal_pos && is_valid_digit)
			{
				++length;
			}
			else if (!post_literal_pos && !is_valid_digit)
			{
				if (base == 16 || !std::isdigit(next_c))
				{
					post_literal_pos = length;
					post_literal_length++;
				}
				else
				{
					error = true;

					const std::size_t pos = static_cast<std::size_t>(data.line_stream.tellg());
					data.metadata.messages().push_back(std::make_shared<error_message>(
						error_id_invalid_digit(), "Invalid digit '"s + next_c + "' in " + base_string() + " literal.",
						generate_line_col(data.source.path(), data.line_line, pos),
						generate_source(data.line, data.line_line, pos, 1)
						));
				}
			}
			else
			{
				if (!post_literal_pos)
				{
					post_literal_pos = length;
				}
				post_literal_length++;
			}
		}
		
		if (length == 2 && !error)
		{
			data.metadata.messages().push_back(std::make_shared<error_message>(
				error_id_invalid_format(), "Invalid "s + base_string() + " literal.",
				generate_line_col(data.source.path(), data.line_line, c_pos + 1),
				generate_source(data.line, data.line_line, c_pos + 1, length)
				));

			return false;
		}
		else if (!error)
		{
			data.tokens.emplace_back(data.line.substr(c_pos, length), token_type, data.line_line, c_pos, prefix,
				data.line.substr(post_literal_pos, post_literal_length));
			return true;
		}
		else
		{
			return false;
		}
	}
}