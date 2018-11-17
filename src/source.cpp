#include <Dlink/source.hpp>

#include <Dlink/decoder.hpp>
#include <Dlink/exception.hpp>
#include <Dlink/lexer.hpp>
#include <Dlink/preprocessor.hpp>

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
		if (state() >= source_state::initialized)
			throw invalid_state("The state must be 'dlink::source_state::initialized' or higher when 'bool dlink::source::decode(dlink::compiler_metadata&)' method is called.");

		return decoder::decode_source(*this, metadata);
	}
	bool dlink::source::preprocess(compiler_metadata& metadata)
	{
		if (state() >= source_state::decoded)
			throw invalid_state("The state must be 'dlink::source_state::decoded' or higher when 'bool dlink::preprocess(dlink::compiler_metadata&)' method is called.");

		return preprocessor::preprocess_source(*this, metadata);
	}
	bool source::lex(compiler_metadata& metadata)
	{
		if (state() >= source_state::preprocessed)
			throw invalid_state("The state must be 'dlink::source_state::preprocessed' or higher when 'bool dlink::source::lex(dlink::compiler_metadata&)' method is called.");

		return lexer::lex_source(*this, metadata);
	}

	bool source::compile_until_preprocessing(compiler_metadata& metadata)
	{
		bool result = decode(metadata);

		if (result)
		{
			result = preprocess(metadata);
		}

		return result;
	}
	bool source::compile_until_lexing(compiler_metadata& metadata)
	{
		bool result = compile_until_preprocessing(metadata);

		if (result)
		{
			result = lex(metadata);
		}

		return result;
	}

	nlohmann::json source::dump() const
	{
		nlohmann::json object;
		object["path"] = path_;

		if (state_ >= source_state::preprocessed)
		{
			object["preprocessed"] = preprocessed_codes_;
		}
		if (state_ >= source_state::lexed)
		{
			object["tokens"] = dump_tokens();
		}

		return object;
	}
	nlohmann::json source::dump_tokens() const
	{
		if (state_ < source_state::lexed)
			throw invalid_state("The state must be 'dlink::source_state::lexed' or higher when 'nlohmann::json dlink::source::dump_tokens(void) const' method is called.");

		nlohmann::json array;
		
		for (const token& token : tokens_)
		{
			array.push_back(token.dump());
		}

		return array;
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
	const std::string& source::preprocessed_codes() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(preprocessed_codes_mutex_);
#endif

		return preprocessed_codes_;
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
	void source::preprocessed_codes(std::string&& new_preprocessed_codes)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(preprocessed_codes_mutex_);
#endif

		codes_ = std::move(new_preprocessed_codes);
		state_ = source_state::preprocessed;
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