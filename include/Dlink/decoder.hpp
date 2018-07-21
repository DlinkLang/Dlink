#ifndef DLINK_HEADER_DECODER_HPP
#define DLINK_HEADER_DECODER_HPP

#include <Dlink/compiler_options.hpp>
#include <Dlink/message.hpp>

#include <string>
#include <vector>

#ifdef DLINK_MULTITHREADING
#	include <mutex>
#endif

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
		void clear();
		bool decode();
		bool decode_singlethread();

	public:
		const compiler_options& options() const noexcept;

		const std::vector<message_ptr>& messages() const noexcept;
		const std::vector<std::string>& results() const noexcept;
		
	private:
		compiler_options options_;
#ifdef DLINK_MULTITHREADING
		std::mutex mutex_;
#endif
		
		std::vector<message_ptr> messages_;
		std::vector<std::string> results_;
	};
}

#endif