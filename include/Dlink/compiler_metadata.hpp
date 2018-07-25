#ifndef DLINK_HEADER_COMPILER_METADATA_HPP
#define DLINK_HEADER_COMPILER_METADATA_HPP

#include <Dlink/compiler_options.hpp>
#include <Dlink/message.hpp>

namespace dlink
{
	class compiler_metadata final
	{
	public:
		compiler_metadata();
		explicit compiler_metadata(const compiler_options& options);
		explicit compiler_metadata(compiler_options&& options);
		compiler_metadata(const compiler_metadata& metadata) = delete;
		compiler_metadata(compiler_metadata&& metadata) noexcept = delete;
		~compiler_metadata() = default;
		
	public:
		compiler_metadata& operator=(const compiler_metadata& metadata) = delete;
		compiler_metadata& operator=(compiler_metadata&& metadata) noexcept = delete;
		bool operator==(const compiler_metadata& metadata) const = delete;
		bool operator!=(const compiler_metadata& metadata) const = delete;

	public:
		const dlink::messages& messages() const noexcept;
		dlink::messages& messages() noexcept;
		const compiler_options& options() const noexcept;
		compiler_options& options() noexcept;

	private:
		dlink::messages messages_;
		compiler_options options_;
	};
}

#endif