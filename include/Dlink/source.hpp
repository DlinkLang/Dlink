#ifndef DLINK_HEADER_SOURCE_HPP
#define DLINK_HEADER_SOURCE_HPP

#include <Dlink/compiler_metadata.hpp>

#include <string>
#include <string_view>

#ifdef DLINK_MULTITHREADING
#	include <mutex>
#endif

namespace dlink
{
	enum class source_state
	{
		empty,
		initialized,
		decoded,
	};

	class source final
	{
		friend class decoder;

	public:
		source(const std::string_view& path);
		source(const source& source) = delete;
		source(source&& source) noexcept;
		~source() = default;

	public:
		source& operator=(const source& source) = delete;
		source& operator=(source&& source) noexcept;
		bool operator==(const source& source) const = delete;
		bool operator!=(const source& source) const = delete;

	public:
		bool empty() const noexcept;
		source_state state() const noexcept;

		bool decode(compiler_metadata& metadata);

	public:
		const std::string& codes() const noexcept;
		const std::string& path() const noexcept;

	private:
		void codes(std::string&& new_codes);

	private:
		std::string codes_;
		std::string path_;
		source_state state_;

#ifdef DLINK_MULTITHREADING
		mutable std::mutex codes_mutex_;
#endif
	};
}

#endif