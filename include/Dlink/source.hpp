#ifndef DLINK_HEADER_SOURCE_HPP
#define DLINK_HEADER_SOURCE_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/token.hpp>
#include <Dlink/extlib/json.hpp>

#include <string>
#include <string_view>
#include <vector>

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
		preprocessed,
		lexed,
	};

	class source final
	{
		friend class decoder;
		friend class preprocessor;
		friend class lexer;

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

		bool decode(compiler_metadata& metadata);
		bool preprocess(compiler_metadata& metadata);
		bool lex(compiler_metadata& metadata);

		bool compile_until_preprocessing(compiler_metadata& metadata);
		bool compile_until_lexing(compiler_metadata& metadata);

		nlohmann::json dump() const;
		nlohmann::json dump_tokens() const;

	public:
		const std::string& codes() const noexcept;
		const std::vector<std::string>& preprocessed_codes() const noexcept;
		const std::string& path() const noexcept;
		const dlink::tokens& tokens() const noexcept;

		source_state state() const noexcept;

	private:
		void codes(std::string&& new_codes);
		void preprocessed_codes(std::vector<std::string>&& new_preprocessed_codes);
		void tokens(dlink::tokens&& new_tokens);

	private:
		std::string codes_;
		std::vector<std::string> preprocessed_codes_;
		std::string path_;
		dlink::tokens tokens_;

		source_state state_;

#ifdef DLINK_MULTITHREADING
		mutable std::mutex codes_mutex_;
		mutable std::mutex preprocessed_codes_mutex_;
		mutable std::mutex tokens_mutex_;
#endif
	};

	using sources = std::vector<source>;
}

#endif