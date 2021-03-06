#include <Dlink/compilation_pipeline.hpp>

#include <Dlink/decoder.hpp>
#include <Dlink/lexer.hpp>
#include <Dlink/preprocessor.hpp>

#include <iostream>
#include <utility>

#ifdef DLINK_MULTITHREADING
#	include <Dlink/threading.hpp>
#endif

namespace dlink
{
	compilation_pipeline::compilation_pipeline(const compiler_options& options)
		: metadata_(options)
	{}
	compilation_pipeline::compilation_pipeline(compiler_options&& options)
		: metadata_(std::move(options))
	{}

	void compilation_pipeline::dump_messages() const
	{
		dump_messages(std::cout);
	}
	void compilation_pipeline::dump_messages(std::ostream& stream) const
	{
		for (const message_ptr& message : metadata_.messages())
		{
			stream << to_string(message) << "\n\n";
		}
	}

	bool compilation_pipeline::decode()
	{
		return decoder::decode(metadata_, sources_);
	}
	bool compilation_pipeline::decode_singlethread()
	{
		return decoder::decode_singlethread(metadata_, sources_);
	}
	bool compilation_pipeline::preprocess()
	{
		return preprocessor::preprocess(metadata_, sources_);
	}
	bool compilation_pipeline::preprocess_singlethread()
	{
		return preprocessor::preprocess_singlethread(metadata_, sources_);
	}
	bool compilation_pipeline::lex()
	{
		return lexer::lex(metadata_, sources_);
	}
	bool compilation_pipeline::lex_singlethread()
	{
		return lexer::lex_singlethread(metadata_, sources_);
	}
	
	bool compilation_pipeline::compile_until_preprocessing()
	{
#ifdef DLINK_MULTITHREADING
		auto compile_multithread = [&](std::size_t begin, std::size_t end) mutable -> bool
		{
			bool result = true;

			for (std::size_t i = begin; i < end; ++i)
			{
				result = result && sources_[i].compile_until_preprocessing(metadata_);
			}

			return result;
		};

		for (const std::string& path : metadata_.options().input_files())
		{
			sources_.emplace_back(path);
		}

		return parallel(compile_multithread, get_threading_info(metadata_));
#else
		return compile_until_preprocessing_singlethread();
#endif
	}
	bool compilation_pipeline::compile_until_preprocessing_singlethread()
	{
		bool result = decode_singlethread();

		if (result)
		{
			result = result && preprocess_singlethread();
		}

		return result;
	}
	bool compilation_pipeline::compile_until_lexing()
	{
#ifdef DLINK_MULTITHREADING
		auto compile_multithread = [&](std::size_t begin, std::size_t end) mutable -> bool
		{
			bool result = true;

			for (std::size_t i = begin; i < end; ++i)
			{
				result = result && sources_[i].compile_until_lexing(metadata_);
			}

			return result;
		};

		for (const std::string& path : metadata_.options().input_files())
		{
			sources_.emplace_back(path);
		}

		return parallel(compile_multithread, get_threading_info(metadata_));
#else
		return compile_until_lexing_singlethread();
#endif
	}
	bool compilation_pipeline::compile_until_lexing_singlethread()
	{
		bool result = compile_until_preprocessing_singlethread();

		if (result)
		{
			result = result && lex_singlethread();
		}

		return result;
	}

	nlohmann::json compilation_pipeline::dump_sources() const
	{
		nlohmann::json object;
		nlohmann::json array;

		for (const source& source : sources_)
		{
			array.push_back(source.dump());
		}

		object["sources"] = array;

		return object;
	}

	const compiler_metadata& compilation_pipeline::metadata() const noexcept
	{
		return metadata_;
	}
	compiler_metadata& compilation_pipeline::metadata() noexcept
	{
		return metadata_;
	}
	const std::vector<source>& compilation_pipeline::sources() const noexcept
	{
		return sources_;
	}
	std::vector<source>& compilation_pipeline::sources() noexcept
	{
		return sources_;
	}
}