#include <Dlink/preprocessor.hpp>

#include <Dlink/exception.hpp>
#include <Dlink/threading.hpp>

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
				result = result && preprocess_source(sources[i], metadata);
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
			result = result && preprocess_source(src, metadata);
		}

		return result;
	}
	bool preprocessor::preprocess_source(source& source, compiler_metadata& metadata)
	{
		if (source.state() >= source_state::decoded)
			throw invalid_state("The state of the argument 'source' must be 'dlink::source_state::decoded' or higher when 'static bool dlink::preprocessor::preprocess_source(dlink::source&, dlink::compiler_metadata&)' method is called.");
	
		// TODO

		return false;
	}
}