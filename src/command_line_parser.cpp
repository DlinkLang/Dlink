#include <Dlink/command_line_parser.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

namespace dlink
{
	bool operator&(command_parameter_format lhs, command_parameter_format rhs) noexcept
	{
		return (static_cast<int>(lhs) & static_cast<int>(rhs)) == static_cast<int>(rhs);
	}
	command_parameter_format operator|(command_parameter_format lhs, command_parameter_format rhs) noexcept
	{
		return static_cast<command_parameter_format>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}
}

namespace dlink
{
	command::command(const std::string_view& command, const std::string_view& description)
		: description_(description)
	{
		parse_command_(command);
	}
	command::command(const std::string_view& command, const std::string_view& description, command_parameter parameter)
		: description_(description), parameter_(parameter)
	{
		parse_command_(command);
	}
	command::command(const std::string_view& command, const std::string_view& description, command_parameter parameter, command_parameter_format format)
		: description_(description), parameter_(parameter), parameter_format_(format)
	{
		parse_command_(command);
	}
	command::command(const command& command) noexcept
		: long_(command.long_), short_(command.short_), description_(command.description_), parameter_(command.parameter_), parameter_format_(command.parameter_format_)
	{}

	command& command::operator=(const command& command) noexcept
	{
		long_ = command.long_;
		short_ = command.short_;
		description_ = command.description_;
		parameter_ = command.parameter_;
		parameter_format_ = command.parameter_format_;

		return *this;
	}

	void command::parse_command_(const std::string_view& command)
	{
		const std::size_t comma_pos = command.find(',');

		if (comma_pos == std::string_view::npos)
		{
			long_ = command;
		}
		else
		{
			long_ = command.substr(0, comma_pos);
			short_ = command.substr(comma_pos + 1);
		}
	}

	std::string_view command::long_command() const noexcept
	{
		return long_;
	}
	std::string_view command::short_command() const noexcept
	{
		return short_;
	}
	std::string_view command::description() const noexcept
	{
		return description_;
	}
	command_parameter command::parameter() const noexcept
	{
		return parameter_;
	}
	command_parameter_format command::parameter_format() const noexcept
	{
		return parameter_format_;
	}
}

namespace dlink::details
{
	command_line_parser_add_options::command_line_parser_add_options(command_line_parser& parser) noexcept
		: parser_(parser)
	{}

	command_line_parser_add_options& command_line_parser_add_options::operator()(const command& command)
	{
		return parser_.add_option(command), *this;
	}
	command_line_parser_add_options& command_line_parser_add_options::operator()(command&& command)
	{
		return parser_.add_option(std::move(command)), *this;
	}
	command_line_parser_add_options& command_line_parser_add_options::operator()(const std::string_view& command, const std::string_view& description)
	{
		return parser_.add_option(command, description), *this;
	}
	command_line_parser_add_options& command_line_parser_add_options::operator()(const std::string_view& command, const std::string_view& description, command_parameter parameter)
	{
		return parser_.add_option(command, description, parameter), *this;
	}
	command_line_parser_add_options& command_line_parser_add_options::operator()(const std::string_view& command, const std::string_view& description, command_parameter parameter,
		command_parameter_format format)
	{
		return parser_.add_option(command, description, parameter, format), *this;
	}
	command_line_parser_add_options& command_line_parser_add_options::operator()()
	{
		return parser_.add_section(), *this;
	}

	command_line_parser& command_line_parser_add_options::parser() const noexcept
	{
		return parser_;
	}
}

namespace dlink
{
	command_line_parser_result::command_line_parser_result(const std::map<const command*, std::vector<std::any>>& result)
		: result_(result)
	{}
	command_line_parser_result::command_line_parser_result(std::map<const command*, std::vector<std::any>>&& result) noexcept
		: result_(std::move(result))
	{}
	command_line_parser_result::command_line_parser_result(const command_line_parser_result& result)
		: result_(result.result_)
	{}
	command_line_parser_result::command_line_parser_result(command_line_parser_result&& result) noexcept
		: result_(std::move(result.result_))
	{}

	command_line_parser_result& command_line_parser_result::operator=(const command_line_parser_result& result)
	{
		result_ = result.result_;
		return *this;
	}
	command_line_parser_result& command_line_parser_result::operator=(command_line_parser_result&& result) noexcept
	{
		result_ = std::move(result.result_);
		return *this;
	}

	std::size_t command_line_parser_result::count(const std::string_view& command) const
	{
		std::map<const dlink::command*, std::vector<std::any>>::const_iterator iter;

		std::string_view command_real;
		std::string_view(dlink::command::*function)() const noexcept = nullptr;

		if (command[1] == '-') // long
		{
			command_real = command.substr(2);
			function = &dlink::command::long_command;
		}
		else // short
		{
			command_real = command.substr(1);
			function = &dlink::command::short_command;
		}

		iter = std::find_if(result_.begin(), result_.end(),
			[&command_real, &function](const std::pair<const dlink::command*, std::vector<std::any>>& element)
		{
			if (!element.first) return false;

			return (element.first->*function)() == command_real;
		});

		if (iter == result_.end()) return 0;
		else return iter->second.size();
	}
	std::vector<std::any> command_line_parser_result::argument(const std::string_view& command) const
	{
		std::map<const dlink::command*, std::vector<std::any>>::const_iterator iter;

		std::string_view command_real;
		std::string_view(dlink::command::*function)() const noexcept = nullptr;

		if (command[1] == '-') // long
		{
			command_real = command.substr(2);
			function = &dlink::command::long_command;
		}
		else // short
		{
			command_real = command.substr(1);
			function = &dlink::command::short_command;
		}

		iter = std::find_if(result_.begin(), result_.end(),
			[&command_real, &function](const std::pair<const dlink::command*, std::vector<std::any>>& element)
		{
			if (!element.first) return false;

			return (element.first->*function)() == command_real;
		});

		if (iter == result_.end()) return {};
		else return iter->second;
	}
	std::vector<std::any> command_line_parser_result::non_command() const
	{
		std::map<const command*, std::vector<std::any>>::const_iterator iter
			= result_.find(nullptr);

		if (iter != result_.end()) return iter->second;
		else return {};
	}
}

namespace dlink
{
	namespace
	{
		std::string new_line(std::string_view string, std::size_t max_width, std::size_t indent)
		{
			if (string.size() <= max_width) return std::string(string);

			std::string result;

			while (string.size() > max_width)
			{
				const std::size_t space = string.find_last_of(' ', 45);

				result += string.substr(0, space);
				result += '\n' + std::string(indent, ' ');
				string = string.substr(space + 1);
			}

			return result += string;
		}
	}

	std::ostream& operator<<(std::ostream& stream, const command_line_parser& parser)
	{
		stream << "Options:\n\n";

		std::size_t max_length = 0;

		for (auto& section : parser.commands_)
		{
			for (auto& command : section)
			{
				const std::size_t long_len = command.long_command().length();
				const std::size_t short_len = command.short_command().length();
				const std::size_t len = long_len + short_len;

				max_length = std::max(max_length, len + (long_len != len && short_len != len ? 4 : 0)
					+ (command.parameter() != command_parameter::none ? 6 : 0)
					- (command.parameter_format() == command_parameter_format::attached ? 1 : 0));
			}
		}

		for (auto& section : parser.commands_)
		{
			for (auto& command : section)
			{
				const std::size_t long_len = command.long_command().length();
				const std::size_t short_len = command.short_command().length();

				std::string_view arg;
				if (command.parameter() != command_parameter::none)
				{
					const command_parameter_format format = command.parameter_format();

					if (format & command_parameter_format::separated)
					{
						arg = " <arg>";
					}
					else if (format & command_parameter_format::attached)
					{
						arg = "<arg>";
					}
					else
					{
						arg = "=<arg>";
					}
				}
				const std::size_t arg_len = arg.size();

				if (long_len && !short_len)
				{
					stream << "  --" << command.long_command() << arg << std::string(max_length - long_len - arg_len + 1, ' ')
						<< new_line(command.description(), 45, max_length + 5) << '\n';
				}
				else if (!long_len && short_len)
				{
					stream << "  -" << command.short_command() << arg << std::string(max_length - short_len - arg_len + 2, ' ')
						<< new_line(command.description(), 45, max_length + 5) << '\n';
				}
				else
				{
					stream << "  -" << command.short_command() << "(--" << command.long_command() << ')' << arg
						<< std::string(max_length - long_len - short_len - arg_len - 2, ' ')
						<< new_line(command.description(), 45, max_length + 5) << '\n';
				}
			}

			stream << '\n';
		}

		return stream;
	}

	command_line_parser::command_line_parser()
	{
		commands_.emplace_back();
	}
	command_line_parser::command_line_parser(const command_line_parser& parser)
		: commands_(parser.commands_)
	{}
	command_line_parser::command_line_parser(command_line_parser&& parser) noexcept
		: commands_(std::move(parser.commands_))
	{}

	command_line_parser& command_line_parser::operator=(const command_line_parser& parser)
	{
		commands_ = parser.commands_;
		return *this;
	}
	command_line_parser& command_line_parser::operator=(command_line_parser&& parser) noexcept
	{
		commands_ = std::move(parser.commands_);
		return *this;
	}

	void command_line_parser::add_option(const command& command)
	{
		commands_.back().push_back(command);
	}
	void command_line_parser::add_option(command&& command)
	{
		commands_.back().push_back(std::move(command));
	}
	void command_line_parser::add_option(const std::string_view& command, const std::string_view& description)
	{
		commands_.back().emplace_back(command, description);
	}
	void command_line_parser::add_option(const std::string_view& command, const std::string_view& description, command_parameter parameter)
	{
		commands_.back().emplace_back(command, description, parameter);
	}
	void command_line_parser::add_option(const std::string_view& command, const std::string_view& description, command_parameter parameter, command_parameter_format format)
	{
		commands_.back().emplace_back(command, description, parameter, format);
	}
	details::command_line_parser_add_options command_line_parser::add_options() noexcept
	{
		return details::command_line_parser_add_options(*this);
	}
	void command_line_parser::add_section()
	{
		commands_.emplace_back();
	}
	command_line_parser_result command_line_parser::parse(int argc, char** argv) const
	{
		std::map<const command*, std::vector<std::any>> result;

		for (int i = 1; i < argc; ++i)
		{
			std::string_view command = argv[i];

			if (command[0] == '-')
			{
				if (command.size() == 1)
					throw std::string("Error: invalid command format.");

				const dlink::command* matched_command_info = nullptr;
				const dlink::command* first_matched_command_info = nullptr;
				std::string_view(dlink::command::*function)() const noexcept = nullptr;

				if (command[1] == '-')
				{
					if (command.size() == 2)
						throw std::string("Error: invalid command format.");

					command.remove_prefix(2);
					function = &dlink::command::long_command;
				}
				else
				{
					command.remove_prefix(1);
					function = &dlink::command::short_command;
				}

				const std::size_t assign_pos = command.find('=');

				for (const auto& section : commands_)
				{
					for (const auto& command_info : section)
					{
						if (command_info.short_command() == command.substr(0, 1) &&
							function == &dlink::command::short_command)
						{
							first_matched_command_info = &command_info;
						}

						const std::string_view name = (command_info.*function)();

						if ((assign_pos == std::string_view::npos && name == command) ||
							(assign_pos != std::string_view::npos && name == command.substr(0, assign_pos)))
						{
							matched_command_info = &command_info;
						}

						if (first_matched_command_info && matched_command_info) goto br;
					}
				}

				if (matched_command_info || first_matched_command_info) goto br;

				if (function == &dlink::command::long_command)
					throw std::string("Error: unknown command '--") + std::string(command) + "'.";
				else
					throw std::string("Error: unknown command '-") + std::string(command) + "'.";

			br:
				if (first_matched_command_info)
				{
					if (command.size() == 1) goto matched;
					else if (assign_pos != std::string_view::npos && matched_command_info) goto matched;

					if (first_matched_command_info->parameter() != command_parameter::none)
					{
						const command_parameter_format format = first_matched_command_info->parameter_format();

						if (format & command_parameter_format::attached)
						{
							const std::string argument(command.substr(1));

							switch (first_matched_command_info->parameter())
							{
							case command_parameter::string:
								result[first_matched_command_info].push_back(argument);
								break;

							case command_parameter::integer:
								result[first_matched_command_info].push_back(std::stoi(argument));
								break;
							}

							continue;
						}
					}
				}
				if (matched_command_info)
				{
				matched:
					if (matched_command_info->parameter() != command_parameter::none)
					{
						if (assign_pos != std::string_view::npos)
						{
							if ((matched_command_info->parameter_format() & command_parameter_format::assigned) == false)
							{
								if (function == &dlink::command::long_command)
									throw std::string("Error: invalid argument format for '--") + std::string(command) + "'.";
								else
									throw std::string("Error: invalid argument format for '-") + std::string(command) + "'.";
							}
							else if (assign_pos + 1 == command.size())
							{
								command.remove_suffix(1);
								if (function == &dlink::command::long_command)
									throw std::string("Error: no argument for '--") + std::string(command) + "'.";
								else
									throw std::string("Error: no argument for '-") + std::string(command) + "'.";
							}

							const std::string argument(command.substr(assign_pos + 1));

							switch (matched_command_info->parameter())
							{
							case command_parameter::string:
								result[matched_command_info].push_back(argument);
								break;

							case command_parameter::integer:
								result[matched_command_info].push_back(std::stoi(argument));
								break;
							}

							continue;
						}

						if ((matched_command_info->parameter_format() & command_parameter_format::separated) == false)
						{
							if (function == &dlink::command::long_command)
								throw std::string("Error: invalid argument format for '--") + std::string(command) + "'.";
							else
								throw std::string("Error: invalid argument format for '-") + std::string(command) + "'.";
						}

						if (++i >= argc || argv[i][0] == '-')
							throw std::string("Error: no argument for '") + std::string(command) + "'.";

						const std::string argument = argv[i];

						switch (matched_command_info->parameter())
						{
						case command_parameter::string:
							result[matched_command_info].push_back(argument);
							break;

						case command_parameter::integer:
							result[matched_command_info].push_back(std::stoi(argument));
							break;
						}

						continue;
					}
					else
					{
						result[matched_command_info].emplace_back();
						continue;
					}
				}

				throw std::string("Error: invalid argument format for '-") + command[0] + "'.";
			}
			else
			{
				if (!accept_non_command)
					throw std::string("Error: '" + std::string(command) + "' isn't a command.");

				result[nullptr].emplace_back(std::string(command));
			}
		}

		return std::move(result);
	}
}