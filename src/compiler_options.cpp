#include <Dlink/compiler_options.hpp>

#include <iostream>
#include <mutex>

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

		count_of_threads_ = -1;
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
		count_of_threads_ = new_count_of_threads;
	}
}