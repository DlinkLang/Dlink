#ifndef DLINK_HEADER_COMPILATION_PIPELINE_HPP
#define DLINK_HEADER_COMPILATION_PIPELINE_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/compiler_options.hpp>
#include <Dlink/decoder.hpp>
#include <Dlink/lexer.hpp>
#include <Dlink/source.hpp>

#include <ostream>
#include <vector>

namespace dlink
{	
	class compilation_pipeline final
	{
	public:
		compilation_pipeline() = default;
		explicit compilation_pipeline(const compiler_options& options);
		explicit compilation_pipeline(compiler_options&& options);
		compilation_pipeline(const compilation_pipeline& pipeline) = delete;
		compilation_pipeline(compilation_pipeline&& pipeline) noexcept = delete;
		~compilation_pipeline() = default;

	public:
		compilation_pipeline& operator=(const compilation_pipeline& pipeline) = delete;
		compilation_pipeline& operator=(compilation_pipeline&& pipeline) noexcept = delete;
		bool operator==(const compilation_pipeline& pipeline) const = delete;
		bool operator!=(const compilation_pipeline& pipeline) const = delete;

	public:
		void dump_messages() const;
		void dump_messages(std::ostream& stream) const;

		bool decode();
		bool decode_singlethread();
		bool lex();
		bool lex_singlethread();

		bool compile_until_lexing();
		bool compile_until_lexing_singlethread();

	public:
		const compiler_metadata& metadata() const noexcept;
		compiler_metadata& metadata() noexcept;
		const std::vector<source>& sources() const noexcept;
		std::vector<source>& sources() noexcept;

	private:
		compiler_metadata metadata_;
		std::vector<source> sources_;
	};
}

#endif