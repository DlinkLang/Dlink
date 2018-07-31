#ifndef DLINK_HEADER_THREADPOOL_HPP
#define DLINK_HEADER_THREADPOOL_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/extlib/threadpool/ThreadPool.hpp>

#include <cstddef>
#include <memory>
#include <utility>

namespace dlink
{
	struct threadpool_info final
	{
		std::size_t average = 0;
		std::size_t remainder = 0;
		std::size_t size = 0;
	};

	using threadpool = ThreadPool;
	using threadpool_ptr = std::unique_ptr<threadpool>;

	std::pair<threadpool_ptr, threadpool_info> make_threadpool(const compiler_metadata& metadata);
}

#endif