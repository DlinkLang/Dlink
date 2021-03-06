#include <Dlink/preprocessor.hpp>

#include <Dlink/encoding.hpp>
#include <Dlink/exception.hpp>
#include <Dlink/utility.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#ifdef DLINK_MULTITHREADING
#	include <Dlink/threading.hpp>
#endif

namespace dlink
{
	bool preprocessor::preprocess(compiler_metadata& metadata, std::vector<source>& sources)
	{
#ifdef DLINK_MULTITHREADING
		auto preprocess_multithread = [&](std::size_t begin, std::size_t end) -> bool
		{
			bool result = true;

			for (std::size_t i = begin; i < end; ++i)
			{
				result = preprocess_source(sources[i], metadata) && result;
			}

			return result;
		};

		return parallel(preprocess_multithread, get_threading_info(metadata));
#else
		return preprocess_singlethread(metadata, sources);
#endif
	}
	bool preprocessor::preprocess_singlethread(compiler_metadata& metadata, std::vector<source>& sources)
	{
		bool result = true;

		for (source& src : sources)
		{
			result = preprocess_source(src, metadata) && result;
		}

		return result;
	}
	bool preprocessor::preprocess_source(source& source, compiler_metadata& metadata)
	{
		if (source.state() < source_state::decoded)
			throw invalid_state("The state of the argument 'source' must be 'dlink::source_state::decoded' or higher when 'static bool dlink::preprocessor::preprocess_source(dlink::source&, dlink::compiler_metadata&)' method is called.");
		
		using memstream = boost::iostreams::stream<boost::iostreams::basic_array_source<char>>;

		std::vector<std::string> result;

		memstream stream(const_cast<char*>(source.codes().c_str()), source.codes().length());
		std::size_t line = 0;
		std::string_view current_line;
		bool ok = true;

#define continue_with_append										\
		{															\
			if (ok)													\
			{														\
				result.emplace_back(current_line);					\
				continue;											\
			}														\
		}

		while (getline(stream, source.codes().c_str(), current_line))
		{
			++line;

			const std::size_t length = current_line.size();
			memstream line_stream(current_line.data(), length);

			char next_c;

			while (is_whitespace(line_stream, next_c));
			if (line_stream.eof()) continue_with_append;
			if (next_c != '#') continue_with_append;



			const std::size_t offset = static_cast<std::size_t>(line_stream.tellg());
			if (offset >= length)
			{
				metadata.messages().push_back(std::make_shared<error_message>(
					1100, message_data::def.error(1100)(),
					generate_line_col(source.path(), line, offset),
					generate_source(current_line, line, offset, 1)
					));
				ok = false;
				continue;
			}

			const std::string_view other = current_line.substr(offset);
			const std::size_t first_space_pos = other.find(' ');
			const std::string_view type = other.substr(0, first_space_pos);
			
			bool loop_error = false;
			std::size_t index = 0;

			for (char c : type)
			{
				if (!isalpha(c))
				{
					metadata.messages().push_back(std::make_shared<error_message>(
						1101, message_data::def.error(1101)(),
						generate_line_col(source.path(), line, offset + index + 1),
						generate_source(current_line, line, offset + index + 1, 1)
						));
					ok = false;
					loop_error = true;
				}

				++index;
			}

			if (loop_error) continue;
			loop_error = false;

			if (type == "error")
			{
				if (first_space_pos == std::string_view::npos ||
					first_space_pos == other.size() - 1)
				{
					metadata.messages().push_back(std::make_shared<error_message>(
						1103, message_data::def.error(1103)(),
						generate_line_col(source.path(), line, offset),
						generate_source(current_line, line, offset, 6)
						));
				}
				else
				{
					const std::string_view message = other.substr(first_space_pos + 1);

					metadata.messages().push_back(std::make_shared<error_message>(
						1104, message_data::def.error(1104)(message),
						generate_line_col(source.path(), line, offset),
						generate_source(current_line, line, offset, message.size() + 7)
						));
				}

				ok = false;
			}
			else if (type == "warning")
			{
				if (first_space_pos == std::string_view::npos ||
					first_space_pos == other.size() - 1)
				{
					metadata.messages().push_back(std::make_shared<warning_message>(
						1100, message_data::def.warning(1100)(),
						generate_line_col(source.path(), line, offset),
						generate_source(current_line, line, offset, 8)
						));
				}
				else
				{
					const std::string_view message = other.substr(first_space_pos + 1);

					metadata.messages().push_back(std::make_shared<warning_message>(
						1101, message_data::def.warning(1101)(message),
						generate_line_col(source.path(), line, offset),
						generate_source(current_line, line, offset, message.size() + 9)
						));
				}
			}
			else
			{
				metadata.messages().push_back(std::make_shared<error_message>(
					1105, message_data::def.error(1105)(),
					generate_line_col(source.path(), line, offset),
					generate_source(current_line, line, offset, type.size() + 1)
					));

				ok = false;
			}
		}

#undef continue_with_append

		if (ok)
		{
			source.preprocessed_codes(std::move(result));
		}

		return ok;
	}
}

namespace dlink
{
	macro_base::macro_base(const std::string_view& name, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line) noexcept
		: name_(name), scope_begin_line_data_(scope_begin_line_data), scope_begin_line_(scope_begin_line)
	{}
	macro_base::macro_base(const std::string_view& name, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line,
						   const std::string_view& scope_end_line_data, std::size_t scope_end_line) noexcept
		: name_(name), scope_begin_line_data_(scope_begin_line_data), scope_begin_line_(scope_begin_line),
		scope_end_line_data_(scope_end_line_data), scope_end_line_(scope_end_line)
	{}
	macro_base::macro_base(const macro_base& macro) noexcept
		: name_(macro.name_), scope_begin_line_data_(macro.scope_begin_line_data_), scope_begin_line_(macro.scope_begin_line_),
		scope_end_line_data_(macro.scope_end_line_data_), scope_end_line_(macro.scope_end_line_)
	{}

	macro_base& macro_base::operator=(const macro_base& macro) noexcept
	{
		name_ = macro.name_;
		scope_begin_line_data_ = macro.scope_begin_line_data_;
		scope_begin_line_ = macro.scope_begin_line_;
		scope_end_line_data_ = macro.scope_end_line_data_;
		scope_end_line_ = macro.scope_end_line_;

		return *this;
	}

	std::string_view macro_base::name() const noexcept
	{
		return name_;
	}
	void macro_base::name(const std::string_view& new_name) noexcept
	{
		name_ = new_name;
	}
	std::string_view macro_base::scope_begin_line_data() const noexcept
	{
		return scope_begin_line_data_;
	}
	void macro_base::scope_begin_line_data(const std::string_view& new_scope_begin_line_data) noexcept
	{
		scope_begin_line_data_ = new_scope_begin_line_data;
	}
	std::string_view macro_base::scope_end_line_data() const noexcept
	{
		return scope_end_line_data_;
	}
	void macro_base::scope_end_line_data(const std::string_view& new_scope_end_line_data) noexcept
	{
		scope_end_line_data_ = new_scope_end_line_data;
	}
	std::size_t macro_base::scope_begin_line() const noexcept
	{
		return scope_begin_line_;
	}
	void macro_base::scope_begin_line(std::size_t new_scope_begin_line) noexcept
	{
		scope_begin_line_ = new_scope_begin_line;
	}
	std::size_t macro_base::scope_end_line() const noexcept
	{
		return scope_end_line_;
	}
	void macro_base::scope_end_line(std::size_t new_scope_end_line) noexcept
	{
		scope_end_line_ = new_scope_end_line;
	}
}

namespace dlink
{
	macro::macro(const std::string_view& name, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line) noexcept
		: macro_base(name, scope_begin_line_data, scope_begin_line)
	{}
	macro::macro(const std::string_view& name, const std::string_view& replaced, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line) noexcept
		: macro_base(name, scope_begin_line_data, scope_begin_line), replaced_(replaced)
	{}
	macro::macro(const std::string_view& name,
				 const std::string_view& scope_begin_line_data, std::size_t scope_begin_line,
				 const std::string_view& scope_end_line_data, std::size_t scope_end_line) noexcept
		: macro_base(name, scope_begin_line_data, scope_begin_line, scope_end_line_data, scope_end_line)
	{}
	macro::macro(const std::string_view& name, const std::string_view& replaced,
				 const std::string_view& scope_begin_line_data, std::size_t scope_begin_line,
				 const std::string_view& scope_end_line_data, std::size_t scope_end_line) noexcept
		: macro_base(name, scope_begin_line_data, scope_begin_line, scope_end_line_data, scope_end_line), replaced_(replaced)
	{}
	macro::macro(const macro& macro) noexcept
		: macro_base(macro), replaced_(macro.replaced_)
	{}

	macro& macro::operator=(const macro& macro) noexcept
	{
		macro_base::operator=(macro);
		replaced_ = macro.replaced_;

		return *this;
	}

	bool macro::need_arguments() const noexcept
	{
		return false;
	}
	std::string macro::replaced(const std::string_view&) const
	{
		return std::string(replaced_);
	}
	void macro::replaced(const std::string_view& new_replaced) noexcept
	{
		replaced_ = new_replaced;
	}
}