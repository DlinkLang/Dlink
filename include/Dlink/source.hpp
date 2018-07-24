#ifndef DLINK_HEADER_SOURCE_HPP
#define DLINK_HEADER_SOURCE_HPP

#include <Dlink/message.hpp>

#include <string>
#include <string_view>

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
	public:
		source(const std::string_view& path, const messages_ptr& messages);
		source(const source& source) = delete;
		source(source&& source) noexcept;
		~source() = default;

	public:
		source& operator=(const source& source) = delete;
		source& operator=(source&& source) noexcept;
		bool operator==(const source& source) const = delete;
		bool operator!=(const source& source) const = delete;

	public:
		source_state state() const noexcept;

	public:
		const std::string& codes() const noexcept;
		const std::string& path() const noexcept;
		
		const messages_ptr& messages() const noexcept;
		messages_ptr& messages() noexcept;

	private:
		std::string codes_;
		std::string path_;
		source_state state_;

		messages_ptr messages_;
	};
}

#endif