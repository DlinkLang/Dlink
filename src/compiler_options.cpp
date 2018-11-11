#include <Dlink/compiler_options.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>

namespace dlink
{
	compiler_options::compiler_options(const compiler_options& options)
		: help_(options.help_), version_(options.version_),
#ifdef DLINK_MULTITHREADING
		count_of_threads_(options.count_of_threads_),
#endif
		input_files_(options.input_files_), output_file_(options.output_file_),
		input_encoding_(options.input_encoding_)
	{}
	compiler_options::compiler_options(compiler_options&& options) noexcept
		: help_(options.help_), version_(options.version_),
#ifdef DLINK_MULTITHREADING
		count_of_threads_(options.count_of_threads_),
#endif
		input_files_(std::move(options.input_files_)), output_file_(std::move(options.output_file_)),
		input_encoding_(std::move(options.input_encoding_))
	{
		options.moved_();
	}

	compiler_options& compiler_options::operator=(const compiler_options& options)
	{
		help_ = options.help_;
		version_ = options.version_;

#ifdef DLINK_MULTITHREADING
		count_of_threads_ = options.count_of_threads_;
#endif
		input_files_ = options.input_files_;
		output_file_ = options.output_file_;

		input_encoding_ = options.input_encoding_;

		return *this;
	}
	compiler_options& compiler_options::operator=(compiler_options&& options) noexcept
	{
		help_ = options.help_;
		version_ = options.version_;

#ifdef DLINK_MULTITHREADING
		count_of_threads_ = options.count_of_threads_;
#endif
		input_files_ = std::move(options.input_files_);
		output_file_ = std::move(options.output_file_);

		input_encoding_ = std::move(options.input_encoding_);

		options.moved_();

		return *this;
	}

	void compiler_options::clear() noexcept
	{
		input_files_.clear();
		output_file_.clear();

		input_encoding_ = encoding::none;

		moved_();
	}

	void compiler_options::add_input(const std::string& string)
	{
		input_files_.push_back(string);
	}
	void compiler_options::remove_input(const std::string& string)
	{
		std::vector<std::string>::iterator iter =
			std::find(input_files_.begin(), input_files_.end(), string);

		if (iter == input_files_.end())
			throw std::invalid_argument("Failed to remove the argument 'string'.");
	}

	void compiler_options::moved_() noexcept
	{
		help_ = false;
		version_ = false;

#ifdef DLINK_MULTITHREADING
		count_of_threads_ = 0;
#endif
	}

	bool compiler_options::help() const noexcept
	{
		return help_;
	}
	void compiler_options::help(bool new_help) noexcept
	{
		help_ = new_help;
	}
	bool compiler_options::version() const noexcept
	{
		return version_;
	}
	void compiler_options::version(bool new_version) noexcept
	{
		version_ = new_version;
	}

	std::int32_t compiler_options::count_of_threads() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		return count_of_threads_;
#else
		return 1;
#endif
	}
#ifdef DLINK_MULTITHREADING
	void compiler_options::count_of_threads(std::int32_t new_count_of_threads) noexcept
	{
		new_count_of_threads = std::clamp(new_count_of_threads, 0, max_count_of_threads);

		count_of_threads_ = new_count_of_threads;
	}
#endif
	const std::vector<std::string>& compiler_options::input_files() const noexcept
	{
		return input_files_;
	}
	const std::string& compiler_options::output_file() const noexcept
	{
		return output_file_;
	}
	void compiler_options::output_file(const std::string_view& new_output_file)
	{
		output_file_ = new_output_file;
	}

	encoding compiler_options::input_encoding() const noexcept
	{
		return input_encoding_;
	}
	void compiler_options::input_encoding(encoding new_encoding_type) noexcept
	{
		input_encoding_ = new_encoding_type;
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
	command::command(const command& command) noexcept
		: long_(command.long_), short_(command.short_), description_(command.description_), parameter_(command.parameter_)
	{}

	command& command::operator=(const command& command) noexcept
	{
		long_ = command.long_;
		short_ = command.short_;
		description_ = command.description_;
		parameter_ = command.parameter_;

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

				max_length = std::max(max_length, len + (long_len != len && short_len != len ? 4 : 0) + (command.parameter() != command_parameter::none ? 4 : 0));
			}
		}
		
		for (auto& section : parser.commands_)
		{
			for (auto& command : section)
			{
				const std::size_t long_len = command.long_command().length();
				const std::size_t short_len = command.short_command().length();

				const std::string_view arg = command.parameter() != command_parameter::none ? " arg" : "";
				const std::size_t arg_len = arg.length();

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

				for (const auto& section : commands_)
				{
					for (const auto& command_info : section)
					{
						if ((command_info.*function)() == command)
						{
							matched_command_info = &command_info;
							goto br;
						}
					}
				}

				if (function == &dlink::command::long_command)
					throw std::string("Error: unknown command '--") + std::string(command) + "'.";
				else
					throw std::string("Error: unknown command '-") + std::string(command) + "'.";

			br:
				if (matched_command_info->parameter() != command_parameter::none)
				{
					if (++i >= argc || argv[i][0] == '-')
						throw std::string("Error: no argument for '" + std::string(command) + "'.");

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
				}
				else
				{
					result[matched_command_info].emplace_back();
				}
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

namespace dlink
{
	bool parse_command_line(int argc, char** argv, compiler_options& options)
	{
		return parse_command_line(std::cout, argc, argv, options);
	}
	bool parse_command_line(std::ostream& stream, int argc, char** argv, compiler_options& options)
	{
		if (argc < 2)
		{
			stream << "Error: no input files.\n\n";

			return false;
		}

		options.clear();

		command_line_parser parser;
		parser.add_options()
			("help", "Display command-line options.")
			("version", "Display compiler version information.")
			()
#ifdef DLINK_MULTITHREADING
			("jobs,j", "Set the maximum number of threads to use when compiling.", command_parameter::integer)
#endif
			(",o", "Place the output into 'arg'.", command_parameter::string)
			()
			(",finput-encoding", "Set the input encoding.", command_parameter::string);
		parser.accept_non_command = true;

		try
		{
			const command_line_parser_result result = parser.parse(argc, argv);
			
			// Check
			if (result.count("--help"))
			{
				options.help(true);
			}
			if (result.count("--version"))
			{
				options.version(true);
			}

			std::string input_encoding_temp;

			std::size_t temp = 0;

#ifdef DLINK_MULTITHREADING
			temp = result.count("-j");
			if (temp)
			{
				if (temp >= 2)
				{
					stream << "Error: '-j' was used more than once.\n\n";
					return false;
				}

				const std::int32_t count = std::clamp(std::any_cast<int>(result.argument("-j").front()), 0, compiler_options::max_count_of_threads);
				options.count_of_threads(count);
			}
#endif
			temp = result.count("-o");
			if (temp)
			{
				if (temp >= 2)
				{
					stream << "Error: '-o' was used more than once.\n\n";
					return false;
				}

				options.output_file(std::any_cast<std::string>(result.argument("-o").front()));
			}

			temp = result.count("-finput-encoding");
			if (temp)
			{
				if (temp >= 2)
				{
					stream << "Error: '-finput-encoding' was used more than once.\n\n";
					return false;
				}

				std::string encoding = std::any_cast<std::string>(result.argument("-finput-encoding").front());
				std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);

				if (encoding == "utf8" || encoding == "utf-8" || encoding == "u8")
				{
					options.input_encoding(encoding::utf8);
				}
				else if (encoding == "utf16" || encoding == "utf-16" || encoding == "u16" ||
					encoding == "utf16le" || encoding == "utf-16le" || encoding == "u16le")
				{
					options.input_encoding(encoding::utf16);
				}
				else if (encoding == "utf16be" || encoding == "utf-16be" || encoding == "u16be")
				{
					options.input_encoding(encoding::utf16be);
				}
				else if (encoding == "utf32" || encoding == "utf-32" || encoding == "u32" ||
					encoding == "utf32le" || encoding == "utf-32le" || encoding == "u32le")
				{
					options.input_encoding(encoding::utf32);
				}
				else if (encoding == "utf32be" || encoding == "utf-32be" || encoding == "u32be")
				{
					options.input_encoding(encoding::utf32be);
				}
				else
				{
					input_encoding_temp = encoding;
				}
			}

			const std::vector<std::any> input = result.non_command();
			for (const std::any& file : input)
			{
				options.add_input(std::any_cast<std::string>(file));
			}

			// Run
			if (options.help())
			{
				stream << "Usage: " << argv[0] << " [options...] files...\n";
				stream << parser;

				return false;
			}
			else if (options.version())
			{
				stream << "Dlink Official Compiler " << program::version << '\n'
					   << "(C) 2018. kmc7468 All rights reserved.\n\n"
						  "https://github.com/DlinkLang/Dlink\n"
						  "This is an open source software.\n"
						  "The sources are licensed under The MIT License.\n\n";

				return false;
			}

			if (options.input_files().size() != 0)
			{
				const std::vector<std::string>::const_iterator duplicate_iter =
					std::unique(const_cast<std::vector<std::string>&>(options.input_files()).begin(),
						const_cast<std::vector<std::string>&>(options.input_files()).end());

				if (duplicate_iter != options.input_files().end())
				{
					stream << "Error: duplicate input files('" << *duplicate_iter << "').\n\n";

					return false;
				}
			}
			else if (options.input_files().size() == 0)
			{
				stream << "Error: no input files.\n\n";

				return false;
			}

			if (!input_encoding_temp.empty())
			{
				stream << "Error: the argument ('" << input_encoding_temp << "') for option '--input-encoding' is invalid";

				if (input_encoding_temp.find('8') != std::string::npos)
				{
					stream << "; did you mean 'utf8'?\n\n";
				}
				else if (input_encoding_temp.find("16") != std::string::npos)
				{
					stream << "; did you mean 'utf16'?\n\n";
				}
				else if (input_encoding_temp.find("32") != std::string::npos)
				{
					stream << "; did you mean 'utf32'?\n\n";
				}
				else
				{
					stream << ".\n\n";
				}

				return false;
			}

			return true;
		}
		catch (const std::string& error)
		{
			stream << error << "\n\n";
			return false;
		}
	}
}