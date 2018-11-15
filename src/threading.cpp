#include <Dlink/threading.hpp>

#include <algorithm>
#include <thread>

namespace dlink
{
	threading_info get_threading_info(const compiler_metadata& metadata)
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

		const std::size_t average = input_files_size / count_of_threads;
		const std::size_t remainder = input_files_size % count_of_threads;

		return { average, remainder, count_of_threads };
	}
}