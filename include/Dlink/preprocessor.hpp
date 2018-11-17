#ifndef DLINK_HEADER_PREPROCESSOR_HPP
#define DLINK_HEADER_PREPROCESSOR_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/source.hpp>

#include <vector>

namespace dlink
{
	class preprocessor final
	{
	public:
		preprocessor() = delete;
		preprocessor(const preprocessor& preprocessor) = delete;
		preprocessor(preprocessor&& preprocessor) noexcept = delete;
		~preprocessor() = default;

	public:
		preprocessor& operator=(const preprocessor& preprocessor) = delete;
		preprocessor& operator=(preprocessor&& preprocessor) noexcept = delete;
		bool operator==(const preprocessor& preprocessor) const = delete;
		bool operator!=(const preprocessor& preprocessor) const = delete;

	public:
		static bool preprocess(compiler_metadata& metadata, std::vector<source>& sources);
		static bool preprocess_singlethread(compiler_metadata& metadata, std::vector<source>& sources);
		static bool preprocess_source(source& source, compiler_metadata& metadata);
	};
}

#endif