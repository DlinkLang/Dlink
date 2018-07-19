#include <Dlink/decoder.hpp>

namespace dlink
{
	decoder::decoder(const compiler_options& options)
		: options_(options)
	{}

	const compiler_options& decoder::options() const noexcept
	{
		return options_;
	}
}