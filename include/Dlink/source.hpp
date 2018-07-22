#ifndef DLINK_HEADER_SOURCE_HPP
#define DLINK_HEADER_SOURCE_HPP

#include <string>
#include <string_view>

namespace dlink
{
	class source final
	{
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
		const std::string& codes() const noexcept;
		const std::string& path() const noexcept;
		
	private:
		std::string codes_;
		std::string path_;
	};
}

#endif