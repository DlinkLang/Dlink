#include <Dlink/compiler_options.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <boost/program_options.hpp>

namespace dlink
{
	void compiler_options::clear()
	{
		help_ = false;
		version_ = false;

#ifdef DLINK_MULTITHREADING
		count_of_threads_ = 0;
#endif
		input_files_.clear();
		output_file_.clear();
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
}

namespace dlink
{
	bool parse_command_line(int argc, char** argv, compiler_options& option)
	{
		return parse_command_line(std::cout, argc, argv, option);
	}
	bool parse_command_line(std::ostream& stream, int argc, char** argv, compiler_options& option)
	{
		if (argc < 2)
		{
			stream << "Error: no input files.\n\n";

			return false;
		}

		namespace po = boost::program_options;
		option.clear();

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

		po::options_description hidden;
		hidden.add_options()
			("input-file", po::value<std::vector<std::string>>(), "");

		po::positional_options_description p;
		p.add("input-file", -1);

		po::options_description visible("Options");
		visible.add(generic).add(config);

		po::options_description description;
		description.add(generic).add(config).add(hidden);

		try
		{
			po::variables_map map;
			po::store(po::command_line_parser(argc, argv).
				options(description).positional(p).run(), map);
			po::notify(map);
			
			if (map.count("help"))
			{
				option.help(true);
			}
			if (map.count("version"))
			{
				option.version(true);
			}

#ifdef DLINK_MULTITHREADING
			if (map.count("job"))
			{
				const std::int32_t count =
					std::clamp(map["job"].as<std::int32_t>(), 0, compiler_options::max_count_of_threads);

				option.count_of_threads(count);
			}
#endif
			if (map.count("output"))
			{
				option.output_file(map["output"].as<std::string>());
			}
			if (map.count("input-file"))
			{
				std::vector<std::string> files = map["input-file"].as<std::vector<std::string>>();
				
				for (const std::string& file : files)
				{
					option.add_input(file);
				}
			}

			if (option.help())
			{
				stream << "Usage: " << argv[0] << " [options...] files...\n";
				stream << visible << '\n';

				return false;
			}
			else if (option.version())
			{
				stream << "Dlink Official Compiler " << program::version << '\n'
					   << "(C) 2018. kmc7468 All rights reserved.\n\n"
						  "https://github.com/DlinkLang/Dlink\n"
						  "This is an open source software.\n"
						  "The sources are licensed under The MIT License.\n\n";

				return false;
			}
			else if (option.input_files().size())
			{
				const std::vector<std::string>::const_iterator duplicate_iter =
					std::unique(const_cast<std::vector<std::string>&>(option.input_files()).begin(),
								const_cast<std::vector<std::string>&>(option.input_files()).end());

				if (duplicate_iter != option.input_files().end())
				{
					stream << "Error: duplicate input files('" << *duplicate_iter << "').\n\n";

					return false;
				}
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