#ifndef DLINK_HEADER_COMPILER_OPTIONS_HPP
#define DLINK_HEADER_COMPILER_OPTIONS_HPP

#include <Dlink/encoding.hpp>

#include <any>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#define DLINK_MAJOR (1)
#define DLINK_MINOR (0)
#define DLINK_PATCH (0)
#define DLINK_VERSION ("1.0.0")

namespace dlink
{
	namespace program
	{
		static constexpr int major = DLINK_MAJOR;
		static constexpr int minor = DLINK_MINOR;
		static constexpr int patchlevel = DLINK_PATCH;
		static constexpr const char* version = DLINK_VERSION;
	}

	class compiler_options final
	{
	public:
		compiler_options() = default;
		compiler_options(const compiler_options& options);
		compiler_options(compiler_options&& options) noexcept;
		~compiler_options() = default;

	public:
		compiler_options& operator=(const compiler_options& options);
		compiler_options& operator=(compiler_options&& options) noexcept;
		bool operator==(const compiler_options& options) const = delete;
		bool operator!=(const compiler_options& options) const = delete;
		
	public:
		void clear() noexcept;

		void add_input(const std::string& string);
		void remove_input(const std::string& string);

	private:
		void moved_() noexcept;

	public:
		bool help() const noexcept;
		void help(bool new_help) noexcept;
		bool version() const noexcept;
		void version(bool new_version) noexcept;

		std::int32_t count_of_threads() const noexcept;
#ifdef DLINK_MULTITHREADING
		void count_of_threads(std::int32_t new_count_of_threads) noexcept;
#endif
		const std::vector<std::string>& input_files() const noexcept;
		const std::string& output_file() const noexcept;
		void output_file(const std::string_view& new_output_file);

		encoding input_encoding() const noexcept;
		void input_encoding(encoding new_encoding_type) noexcept;

	private:
		bool help_ = false;
		bool version_ = false;

#ifdef DLINK_MULTITHREADING
		std::int32_t count_of_threads_ = 1;
#endif
		std::vector<std::string> input_files_;
		std::string output_file_;

		encoding input_encoding_ = encoding::none;

	public:
		static constexpr std::int32_t max_count_of_threads = 128;
	};

	enum class command_parameter
	{
		none,
		string,
		integer,
	};

	class command final
	{
	public:
		command(const std::string_view& command, const std::string_view& description);
		command(const std::string_view& command, const std::string_view& description, command_parameter parameter);
		command(const command& command) noexcept;
		~command() = default;

	public:
		command& operator=(const command& command) noexcept;
		bool operator==(const command& command) const = delete;
		bool operator!=(const command& command) const = delete;

	private:
		void parse_command_(const std::string_view& command);

	public:
		std::string_view long_command() const noexcept;
		std::string_view short_command() const noexcept;
		std::string_view description() const noexcept;
		command_parameter parameter() const noexcept;

	private:
		std::string_view long_;
		std::string_view short_;
		std::string_view description_;
		command_parameter parameter_ = command_parameter::none;
	};

	class command_line_parser;

	namespace details
	{
		class command_line_parser_add_options final
		{
		public:
			command_line_parser_add_options(command_line_parser& parser) noexcept;
			command_line_parser_add_options(const command_line_parser_add_options& parser) = delete;
			command_line_parser_add_options(command_line_parser_add_options&& parser) noexcept = delete;
			~command_line_parser_add_options() = default;

		public:
			command_line_parser_add_options& operator=(const command_line_parser_add_options& parser) = delete;
			command_line_parser_add_options& operator=(command_line_parser_add_options&& parser) noexcept = delete;
			bool operator==(const command_line_parser_add_options& parser) const = delete;
			bool operator!=(const command_line_parser_add_options& parser) const = delete;
			command_line_parser_add_options& operator()(const command& command);
			command_line_parser_add_options& operator()(command&& command);
			command_line_parser_add_options& operator()(const std::string_view& command, const std::string_view& description);
			command_line_parser_add_options& operator()(const std::string_view& command, const std::string_view& description, command_parameter parameter);
			command_line_parser_add_options& operator()();

		public:
			command_line_parser& parser() const noexcept;

		private:
			command_line_parser& parser_;
		};
	}

	class command_line_parser_result final
	{
	public:
		command_line_parser_result(const std::map<command, std::pair<int, std::any>>& result);
		command_line_parser_result(std::map<command, std::pair<int, std::any>>&& result) noexcept;
		command_line_parser_result(const command_line_parser_result& result);
		command_line_parser_result(command_line_parser_result&& result) noexcept;
		~command_line_parser_result() = default;

	public:
		command_line_parser_result& operator=(const command_line_parser_result& result);
		command_line_parser_result& operator=(command_line_parser_result&& result) noexcept;
		bool operator==(const command_line_parser_result& result) const = delete;
		bool operator!=(const command_line_parser_result& result) const = delete;

	public:
		int count(const std::string_view& command) const;
		std::optional<std::any> argument(const std::string_view& command) const;

	private:
		std::map<command, std::pair<int, std::any>> result_;
	};

	class command_line_parser final
	{
		friend std::ostream& operator<<(std::ostream& stream, const command_line_parser& parser);

	public:
		command_line_parser();
		command_line_parser(const command_line_parser& parser);
		command_line_parser(command_line_parser&& parser) noexcept;
		~command_line_parser() = default;
		
	public:
		command_line_parser& operator=(const command_line_parser& parser);
		command_line_parser& operator=(command_line_parser&& parser) noexcept;
		bool operator==(const command_line_parser& parser) const = delete;
		bool operator!=(const command_line_parser& parser) const = delete;

	public:
		void add_option(const command& command);
		void add_option(command&& command);
		void add_option(const std::string_view& command, const std::string_view& description);
		void add_option(const std::string_view& command, const std::string_view& description, command_parameter parameter);
		void add_section();
		details::command_line_parser_add_options add_options() noexcept;

	public:
		bool accept_non_command = false;

	private:
		std::vector<std::vector<command>> commands_;
	};

	bool parse_command_line(int argc, char** argv, compiler_options& options);
	bool parse_command_line(std::ostream& stream, int argc, char** argv, compiler_options& options);
}

#endif