#include <Dlink/source.hpp>

#include <fstream>
#include <utility>

namespace dlink
{
	source::source(const std::string_view& path, const messages_ptr& messages)
		: path_(path), state_(source_state::initialized), messages_(messages)
	{
		if (path.empty())
		{
			state_ = source_state::empty;
			throw std::invalid_argument("The argument 'path' can't be empty.");
		}
		
		if (messages == nullptr)
		{
			state_ = source_state::empty;
			throw std::invalid_argument("The argument 'messages' can't be nullptr.");
		}
	}
	source::source(source&& source) noexcept
		: codes_(std::move(source.codes_)), path_(std::move(source.path_)), state_(source.state_),
		messages_(std::move(source.messages_))
	{
		source.state_ = source_state::empty;
	}

	source& source::operator=(source&& source) noexcept
	{
		codes_ = std::move(source.codes_);
		path_ = std::move(source.path_);
		state_ = std::move(source.state_);
		messages_ = std::move(source.messages_);

		source.state_ = source_state::empty;

		return *this;
	}

	source_state source::state() const noexcept
	{
		return state_;
	}

	const std::string& source::codes() const noexcept
	{
		return codes_;
	}
	const std::string& source::path() const noexcept
	{
		return path_;
	}

	const messages_ptr& source::messages() const noexcept
	{
		return messages_;
	}
	messages_ptr& source::messages() noexcept
	{
		return messages_;
	}
}