#include <Dlink/source.hpp>

#include <Dlink/decoder.hpp>
#include <Dlink/exception.hpp>
#include <Dlink/lexer.hpp>

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

	bool source::decode(compiler_metadata& metadata)
	{
		if (state() != source_state::initialized)
			throw invalid_state("The state must be 'dlink::source_state::initialized' when 'bool dlink::source::decode(void)' method is called.");

		return decoder::decode_source(*this, metadata);
	}
	bool source::lex(compiler_metadata& metadata)
	{
		if (state() != source_state::decoded)
			throw invalid_state("The state must be 'dlink::source_state::decoded' when 'bool dlink::source::lex(void)' method is called.");

		return lexer::lex_source(*this, metadata);
	}

	const std::string& source::path() const noexcept
	{
		return path_;
	}
	const std::string& source::codes() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(codes_mutex_);
#endif

		return codes_;
	}
	const dlink::tokens& source::tokens() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(tokens_mutex_);
#endif

		return tokens_;
	}

	source_state source::state() const noexcept
	{
		return state_;
	}

	void source::codes(std::string&& new_codes)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(codes_mutex_);
#endif

		codes_ = std::move(new_codes);
		state_ = source_state::decoded;
	}
	void source::tokens(dlink::tokens&& new_tokens)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(tokens_mutex_);
#endif

		tokens_ = std::move(new_tokens);
		state_ = source_state::lexed;
	}
}