#include <Dlink/source.hpp>

#include <fstream>
#include <utility>

namespace dlink
{
	source::source(const std::string_view& path)
		: path_(path)
	{
		// TODO
	}
	source::source(source&& source) noexcept
		: codes_(std::move(source.codes_)), path_(std::move(source.path_))
	{}

	source& source::operator=(source&& source) noexcept
	{
		codes_ = std::move(source.codes_);
		path_ = std::move(source.path_);

		return *this;
	}

	const std::string& source::codes() const noexcept
	{
		return codes_;
	}
	const std::string& source::path() const noexcept
	{
		return path_;
	}
}