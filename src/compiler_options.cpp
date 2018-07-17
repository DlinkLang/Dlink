#include <Dlink/compiler_options.hpp>

#include <algorithm>
#include <iostream>
#include <mutex>

#include <boost/program_options.hpp>

namespace dlink
{
	void compiler_options::clear()
	{
#ifdef DLINK_MULTITHREADING
		static std::mutex mutex;
		std::lock_guard<std::mutex> guard(mutex);
#endif

		help_ = false;
		version_ = false;

		count_of_threads_ = 0;
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
		return count_of_threads_;
	}
	void compiler_options::count_of_threads(std::int32_t new_count_of_threads) noexcept
	{
		new_count_of_threads = std::clamp(new_count_of_threads, 0, max_count_of_threads);

		count_of_threads_ = new_count_of_threads;
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

		po::options_description description("Options");
		description.add_options()
			("help", "Display command-line options.")
			("version", "Display compiler version information.")
			("job,j", po::value<std::int32_t>(), "Set the maximum number of threads to use when compiling.");

		try
		{
			po::variables_map map;
			po::store(po::parse_command_line(argc, argv, description), map);
								
			if (map.count("help"))
			{
				option.help(true);
			}
			if (map.count("version"))
			{
				option.version(true);
			}

			if (map.count("job"))
			{
				const std::int32_t count =
					std::clamp(map["job"].as<std::int32_t>(), 0, compiler_options::max_count_of_threads);

				option.count_of_threads(count);
			}

			if (option.help())
			{
				stream << "Usage: " << argv[0] << " [options...] files...\n";
				stream << description << '\n';

				return false;
			}
			else if (option.version())
			{
				stream << "Dlink Official Compiler " << program::version << '\n'
					   << "(C) 2018. kmc7468 All rights reserved.\n\n"
						  "https://github.com/DlinkLang/Dlink\n"
						  "This is an open source software.\n"
						  "nThe sources are licensed under The MIT License.\n\n";

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