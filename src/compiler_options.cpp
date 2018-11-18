#include <Dlink/compiler_options.hpp>

#include <Dlink/command_line_parser.hpp>
#include <Dlink/lexer.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

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

		macros_ = options.macros_;

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

		macros_ = std::move(options.macros_);

		input_encoding_ = std::move(options.input_encoding_);

		options.moved_();

		return *this;
	}

	void compiler_options::clear() noexcept
	{
		input_files_.clear();
		output_file_.clear();

		macros_.clear();

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
	void compiler_options::add_macro(const std::string& macro)
	{
		macros_[macro];
	}
	void compiler_options::add_macro(const std::string& macro, const std::string& replaced)
	{
		macros_[macro] = replaced;
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

	const std::map<std::string, std::string>& compiler_options::macros() const noexcept
	{
		return macros_;
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
			(",j", "Set the maximum number of threads to use when compiling.", command_parameter::integer, command_parameter_format::all)
#endif
			(",o", "Place the output into 'arg'.", command_parameter::string, command_parameter_format::all)
			()
			(",D", "Define the macro for preprocessor.", command_parameter::string, command_parameter_format::separated | command_parameter_format::attached)
			()
			(",finput-encoding", "Set the input encoding.", command_parameter::string, command_parameter_format::separated | command_parameter_format::assigned);
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

			std::string macro_dup;

			temp = result.count("-D");
			if (temp)
			{
				std::vector<std::any> arguments = result.argument("-D");

				for (std::any& argument : arguments)
				{
					const std::string argument_str = std::any_cast<std::string>(argument);
					const std::size_t argument_str_assign_pos = argument_str.find('=');

					if (argument_str_assign_pos == std::string_view::npos)
					{
					replaced_empty:
						if (options.macros().find(argument_str) == options.macros().end())
						{
							options.add_macro(argument_str);
						}
						else
						{
							macro_dup = argument_str;
							break;
						}
					}
					else
					{
						if (argument_str_assign_pos + 1 == argument_str.size()) goto replaced_empty;

						const std::string macro = argument_str.substr(0, argument_str_assign_pos);
						const std::string replaced = argument_str.substr(argument_str_assign_pos + 1);

						if (options.macros().find(macro) == options.macros().end())
						{
							options.add_macro(macro, replaced);
						}
						else
						{
							macro_dup = macro;
							break;
						}
					}
				}
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

			if (!macro_dup.empty())
			{
				stream << "Error: the macro('" + macro_dup + "') is duplicated.\n\n";

				return false;
			}
			for (const auto& [macro, replaced] : options.macros())
			{
				if (lexer::check_invalid_identifier(macro))
				{
					stream << "Error: the macro('" + macro + "') isn't a valid identifier.\n\n";

					return false;
				}
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