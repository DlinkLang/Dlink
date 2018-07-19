#ifndef DLINK_HEADER_DECODER_HPP
#define DLINK_HEADER_DECODER_HPP

#include <Dlink/compiler_options.hpp>

namespace dlink
{
	class decoder final
	{
	public:
		decoder(const compiler_options& options);
		decoder(const decoder& decoder) = delete;
		decoder(decoder&& decoder) noexcept = delete;
		~decoder() = default;

	public:
		decoder& operator=(const decoder& decoder) = delete;
		decoder& operator=(decoder&& decoder) noexcept = delete;
		bool operator==(const decoder& decoder) const = delete;
		bool operator!=(const decoder& decoder) const = delete;

	public:
		const compiler_options& options() const noexcept;

	private:
		compiler_options options_;
	};
}

#endif