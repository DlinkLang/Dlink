#ifndef DLINK_HEADER_PREPROCESSOR_HPP
#define DLINK_HEADER_PREPROCESSOR_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/source.hpp>

#include <cstddef>
#include <string_view>
#include <vector>

namespace dlink
{
	class preprocessor final
	{
	public:
		preprocessor() = delete;
		preprocessor(const preprocessor& preprocessor) = delete;
		preprocessor(preprocessor&& preprocessor) noexcept = delete;
		~preprocessor() = default;

	public:
		preprocessor& operator=(const preprocessor& preprocessor) = delete;
		preprocessor& operator=(preprocessor&& preprocessor) noexcept = delete;
		bool operator==(const preprocessor& preprocessor) const = delete;
		bool operator!=(const preprocessor& preprocessor) const = delete;

	public:
		static bool preprocess(compiler_metadata& metadata, std::vector<source>& sources);
		static bool preprocess_singlethread(compiler_metadata& metadata, std::vector<source>& sources);
		static bool preprocess_source(source& source, compiler_metadata& metadata);
	};

	class macro_base
	{
	public:
		virtual ~macro_base() = default;

	protected:
		macro_base(const std::string_view& name, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line) noexcept;
		macro_base(const std::string_view& name, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line,
				   const std::string_view& scope_end_line_data, std::size_t scope_end_line) noexcept;
		macro_base(const macro_base& macro) noexcept;

	public:
		bool operator==(const macro_base& macro) const = delete;
		bool operator!=(const macro_base& macro) const = delete;

	protected:
		macro_base& operator=(const macro_base& macro) noexcept;

	public:
		virtual bool need_arguments() const noexcept = 0;
		virtual std::string replaced(const std::string_view& arguments) const = 0;

	public:
		std::string_view name() const noexcept;
		void name(const std::string_view& new_name) noexcept;
		std::string_view scope_begin_line_data() const noexcept;
		void scope_begin_line_data(const std::string_view& new_scope_begin_line_data) noexcept;
		std::string_view scope_end_line_data() const noexcept;
		void scope_end_line_data(const std::string_view& new_scope_end_line_data) noexcept;
		std::size_t scope_begin_line() const noexcept;
		void scope_begin_line(std::size_t new_scope_begin_line) noexcept;
		std::size_t scope_end_line() const noexcept;
		void scope_end_line(std::size_t new_scope_end_line) noexcept;

	private:
		std::string_view name_;
		std::string_view scope_begin_line_data_;
		std::string_view scope_end_line_data_;
		std::size_t scope_begin_line_;
		std::size_t scope_end_line_ = 0;
	};

	class macro final : public macro_base
	{
	public:
		macro(const std::string_view& name, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line) noexcept;
		macro(const std::string_view& name, const std::string_view& replaced, const std::string_view& scope_begin_line_data, std::size_t scope_begin_line) noexcept;
		macro(const std::string_view& name,
			  const std::string_view& scope_begin_line_data, std::size_t scope_begin_line,
			  const std::string_view& scope_end_line_data, std::size_t scope_end_line) noexcept;
		macro(const std::string_view& name, const std::string_view& replaced,
			  const std::string_view& scope_begin_line_data, std::size_t scope_begin_line,
			  const std::string_view& scope_end_line_data, std::size_t scope_end_line) noexcept;
		macro(const macro& macro) noexcept;
		virtual ~macro() override = default;

	public:
		macro& operator=(const macro& macro) noexcept;
		bool operator==(const macro& macro) const = delete;
		bool operator!=(const macro& macro) const = delete;

	public:
		virtual bool need_arguments() const noexcept override;
		virtual std::string replaced(const std::string_view&) const override;
		void replaced(const std::string_view& new_replaced) noexcept;
		
	private:
		std::string_view replaced_;
	};
}

#endif