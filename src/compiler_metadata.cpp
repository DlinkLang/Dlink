#include <Dlink/compiler_metadata.hpp>

#include <utility>

namespace dlink
{
	compiler_metadata::compiler_metadata()
	{}
	compiler_metadata::compiler_metadata(const compiler_options& options)
		: options_(options)
	{}
	compiler_metadata::compiler_metadata(compiler_options&& options)
		: options_(std::move(options))
	{}

	const dlink::messages& compiler_metadata::messages() const noexcept
	{
		return messages_;
	}
	dlink::messages& compiler_metadata::messages() noexcept
	{
		return messages_;
	}
	const compiler_options& compiler_metadata::options() const noexcept
	{
		return options_;
	}
	compiler_options& compiler_metadata::options() noexcept
	{
		return options_;
	}
}