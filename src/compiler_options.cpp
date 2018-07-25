#include <Dlink/compiler_options.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <boost/program_options.hpp>

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

		namespace po = boost::program_options;
		options.clear();

		po::options_description generic;
		generic.add_options()
			("help", "Display command-line options.")
			("version", "Display compiler version information.");

		po::options_description config;
		config.add_options()
#ifdef DLINK_MULTITHREADING
			("jobs,j", po::value<std::int32_t>(), "Set the maximum number of threads to use when compiling.")
#endif
			("output,o", po::value<std::string>(), "Place the output into 'arg'.");

		po::options_description flag;
		flag.add_options()
			("input-encoding", po::value<std::string>(), "Set the input encoding.");

		po::options_description hidden;
		hidden.add_options()
			("input-file", po::value<std::vector<std::string>>(), "");

		po::positional_options_description p;
		p.add("input-file", -1);

		po::options_description visible("Options");
		visible.add(generic).add(config).add(flag);

		po::options_description description;
		description.add(visible).add(hidden);

		try
		{
			po::variables_map map;
			po::store(po::command_line_parser(argc, argv).
				options(description).positional(p).run(), map);
			po::notify(map);
			
			//
			// Parse
			//
			// Generic
			if (map.count("help"))
			{
				options.help(true);
			}
			if (map.count("version"))
			{
				options.version(true);
			}

			// Config
#ifdef DLINK_MULTITHREADING
			if (map.count("jobs"))
			{
				const std::int32_t count =
					std::clamp(map["jobs"].as<std::int32_t>(), 0, compiler_options::max_count_of_threads);

				options.count_of_threads(count);
			}
#endif
			if (map.count("output"))
			{
				options.output_file(map["output"].as<std::string>());
			}
			if (map.count("input-file"))
			{
				std::vector<std::string> files = map["input-file"].as<std::vector<std::string>>();
				
				for (const std::string& file : files)
				{
					options.add_input(file);
				}
			}

			// Flag
			std::string input_encoding_temp;
			if (map.count("input-encoding"))
			{
				std::string encoding = map["input-encoding"].as<std::string>();
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

			//
			// Run
			//
			// Generic
			if (options.help())
			{
				stream << "Usage: " << argv[0] << " [options...] files...\n";
				stream << visible << '\n';

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

			// Config
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

			// Flag
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
		catch (const po::error& e)
		{
			stream << "Error: " << e.what() << ".\n\n";
			
			return false;
		}
	}
}