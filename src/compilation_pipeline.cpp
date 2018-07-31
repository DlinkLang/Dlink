#include <Dlink/compilation_pipeline.hpp>

#include <iostream>
#include <utility>

#ifdef DLINK_MULTITHREADING
#	include <Dlink/threading.hpp>

#	include <future>
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
	bool compilation_pipeline::lex()
	{
		return lexer::lex(metadata_, sources_);
	}
	bool compilation_pipeline::lex_singlethread()
	{
		return decoder::decode_singlethread(metadata_, sources_);
	}
	
	bool compilation_pipeline::compile_until_lexing()
	{
#ifdef DLINK_MULTITHREADING
		const std::size_t input_files_size = metadata_.options().input_files().size();

		const threading_info info = get_threading_info(metadata_);

		static auto compile_multithread = [&](std::size_t begin, std::size_t end) -> bool
		{
			bool result = true;

			for (std::size_t i = begin; i < end; ++i)
			{
				result = result && sources_[i].compile_until_lexing(metadata_);
			}

			return result;
		};

		for (std::size_t i = 0; i < input_files_size; ++i)
		{
			sources_.emplace_back(metadata_.options().input_files()[i]);
		}

		std::vector<std::future<bool>> futures;

		for (std::size_t i = 0; i < info.count_of_threads - 1; ++i)
		{
			futures.push_back(
				std::async(compile_multithread,
						   i * info.average, (i + 1) * info.average
				)
			);
		}

		futures.push_back(
			std::async(compile_multithread,
					   (info.count_of_threads - 1) * info.average,
					   info.count_of_threads * info.average + info.remainder)
		);

		bool result = true;

		for (std::future<bool>& future : futures)
		{
			future.wait();
			result = result && future.get();
		}

		return result;
#else
		return compile_until_lexing_singlethread();
#endif
	}
	bool compilation_pipeline::compile_until_lexing_singlethread()
	{
		bool result = decode_singlethread();

		if (result)
		{
			result = result && lex_singlethread();
		}

		return result;
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