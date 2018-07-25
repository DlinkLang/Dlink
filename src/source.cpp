#include <Dlink/source.hpp>

#include <Dlink/decoder.hpp>
#include <Dlink/exception.hpp>

#include <utility>

#ifdef DLINK_MULTITHREADING
#	include <mutex>
#endif

namespace dlink
{
	source::source(const std::string_view& path)
		: path_(path), state_(source_state::initialized)
	{
		if (path.empty())
		{
			state_ = source_state::empty;
			throw std::invalid_argument("The argument 'path' can't be empty.");
		}
	}
	source::source(source&& source) noexcept
		: codes_(std::move(source.codes_)), path_(std::move(source.path_)), state_(source.state_)
	{
		source.state_ = source_state::empty;
	}

	source& source::operator=(source&& source) noexcept
	{
		codes_ = std::move(source.codes_);
		path_ = std::move(source.path_);
		state_ = std::move(source.state_);

		source.state_ = source_state::empty;

		return *this;
	}

	bool source::empty() const noexcept
	{
		return state() == source_state::empty;
	}
	source_state source::state() const noexcept
	{
		return state_;
	}

	bool source::decode(compiler_metadata& metadata)
	{
		if (empty())
			throw invalid_state("The state shouldn't be empty when 'bool dlink::source::decode(void)' method is called.");

		return decoder::decode_source(*this, metadata);
	}

	const std::string& source::codes() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(codes_mutex_);
#endif

		return codes_;
	}
	const std::string& source::path() const noexcept
	{
		return path_;
	}

	void source::codes(std::string&& new_codes)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(codes_mutex_);
#endif

		codes_ = std::move(new_codes);
		state_ = source_state::decoded;
	}
}