#ifndef DLINK_HEADER_THREADING_HPP
#define DLINK_HEADER_THREADING_HPP

#include <Dlink/compiler_metadata.hpp>

#include <cstddef>
#include <future>
#include <type_traits>
#include <utility>
#include <vector>

namespace dlink
{
	struct threading_info final
	{
		std::size_t average = 0;
		std::size_t remainder = 0;
		std::size_t count_of_threads = 0;
	};

	threading_info get_threading_info(const compiler_metadata& metadata);

	template<typename Func_>
	bool parallel(Func_&& function, const threading_info& info, std::size_t offset = 0)
	{
		bool result = true;

		std::vector<std::future<bool>> futures;

		for (std::size_t i = 0; i < info.count_of_threads - 1; ++i)
		{
			futures.push_back(
				std::async(std::forward<Func_>(function),
						   i * info.average + offset,
						   (i + 1) * info.average + offset
			));
		}

		futures.push_back(
			std::async(std::forward<Func_>(function),
					   (info.count_of_threads - 1) * info.average + offset,
					   info.count_of_threads * info.average + info.remainder + offset
		));

		for (auto& future : futures)
		{
			future.wait();
			result = result && future.get();
		}

		return result;
	}
}

#endif