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
			const std::size_t pos = stream.tellg().seekpos();
			
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
		
		using namespace std::string_literals;
		using memorystream = boost::iostreams::stream<boost::iostreams::basic_array_source<char>>;

		std::size_t line = 0;

		memorystream stream(
			const_cast<char*>(source.codes().c_str()), source.codes().length()
		);
		std::unique_ptr<memorystream> line_stream = nullptr;
		std::string_view current_line;

		dlink::tokens tokens;

		bool multiline_comment = false;

		while (getline(stream, source.codes().c_str(), current_line))
		{
			++line;
			line_stream = std::make_unique<memorystream>(current_line.data(), current_line.size());

			const std::fpos_t length = current_line.size();

			while (!line_stream->eof())
			{
				const char c = static_cast<char>(line_stream->get());
				const std::fpos_t i = line_stream->tellg().seekpos();

				if (std::isdigit(c))
				{
					std::string temp = "0";
					int base = 0;

					if (c == '0' && i < length - 2)
					{
						char next_c;
						line_stream->read(&next_c, 1);

						switch (next_c)
						{
						case 'B':
						case 'b':
						{
							temp += next_c;
							base = 2;

							while (!is_whitespace(*line_stream))
							{
								line_stream->read(&next_c, 1);

								if (next_c == '0' || next_c == '1')
								{
									temp += next_c;
								}
								else if (next_c != '_')
								{
									metadata.messages().push_back(std::make_shared<error_message>(
										2000, "Invalid digit '"s + next_c + "' in binary literal.",
										generate_line_col(source.path(), line, static_cast<std::size_t>(line_stream->tellg().seekpos())),
										generate_source(current_line, line, static_cast<std::size_t>(line_stream->tellg().seekpos()), 1)
										));

									return false;
								}
							}

							tokens.emplace_back(temp, token_type::integer_bin, line, i);
						}
						}
					}
				}
			}
		}

		source.tokens(std::move(tokens));
		return true;
	}
}