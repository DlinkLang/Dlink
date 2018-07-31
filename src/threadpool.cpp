#include <Dlink/threadpool.hpp>

#include <algorithm>
#include <thread>

namespace dlink
{
	std::pair<threadpool_ptr, threadpool_info> make_threadpool(const compiler_metadata& metadata)
	{
		const std::size_t input_files_size = metadata.options().input_files().size();
		std::size_t count_of_threads = metadata.options().count_of_threads();

		if (count_of_threads == 0)
		{
			count_of_threads = std::thread::hardware_concurrency();

			if (count_of_threads == 0)
			{
				count_of_threads = 4;
			}
		}

		count_of_threads = std::min(count_of_threads, input_files_size);

		std::size_t average = input_files_size / count_of_threads;
		std::size_t remainder = input_files_size % count_of_threads;

		if (remainder != 0)
		{
			const std::size_t remainder_average = remainder / count_of_threads;
			const std::size_t remainder_remainder = remainder % count_of_threads;

			average += remainder_average;
			remainder = remainder_remainder;
		}

		return { std::make_unique<threadpool>(count_of_threads),
				 threadpool_info{ average, remainder, count_of_threads } };
	}
}