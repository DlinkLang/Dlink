#ifndef DLINK_HEADER_COMMAND_LINE_PARSER_HPP
#define DLINK_HEADER_COMMAND_LINE_PARSER_HPP

#include <any>
#include <map>
#include <ostream>
#include <string_view>
#include <vector>

namespace dlink
{
	enum class command_parameter
	{
		none,
		string,
		integer,
	};

	enum class command_parameter_format
	{
		separated = 0b001, // -a b
		assigned = 0b010,  // -a=b
		attached = 0b100,  // -ab

		all = separated | assigned | attached,
	};

	bool operator&(command_parameter_format lhs, command_parameter_format rhs) noexcept;
	command_parameter_format operator|(command_parameter_format lhs, command_parameter_format rhs) noexcept;

	class command final
	{
	public:
		command(const std::string_view& command, const std::string_view& description);
		command(const std::string_view& command, const std::string_view& description, command_parameter parameter);
		command(const std::string_view& command, const std::string_view& description, command_parameter parameter, command_parameter_format format);
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
		command_parameter_format parameter_format() const noexcept;

	private:
		std::string_view long_;
		std::string_view short_;
		std::string_view description_;
		command_parameter parameter_ = command_parameter::none;
		command_parameter_format parameter_format_ = command_parameter_format::separated;
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
			command_line_parser_add_options& operator()(const std::string_view& command, const std::string_view& description, command_parameter parameter,
				command_parameter_format format);
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
		command_line_parser_result(const std::map<const command*, std::vector<std::any>>& result);
		command_line_parser_result(std::map<const command*, std::vector<std::any>>&& result) noexcept;
		command_line_parser_result(const command_line_parser_result& result);
		command_line_parser_result(command_line_parser_result&& result) noexcept;
		~command_line_parser_result() = default;

	public:
		command_line_parser_result& operator=(const command_line_parser_result& result);
		command_line_parser_result& operator=(command_line_parser_result&& result) noexcept;
		bool operator==(const command_line_parser_result& result) const = delete;
		bool operator!=(const command_line_parser_result& result) const = delete;

	public:
		std::size_t count(const std::string_view& command) const;
		std::vector<std::any> argument(const std::string_view& command) const;
		std::vector<std::any> non_command() const;

	private:
		std::map<const command*, std::vector<std::any>> result_;
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
		void add_option(const std::string_view& command, const std::string_view& description, command_parameter parameter, command_parameter_format format);
		void add_section();
		details::command_line_parser_add_options add_options() noexcept;
		command_line_parser_result parse(int argc, char** argv) const;

	public:
		bool accept_non_command = false;

	private:
		std::vector<std::vector<command>> commands_;
	};
}

#endif