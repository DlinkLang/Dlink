#ifndef DLINK_HEADER_THREADING_HPP
#define DLINK_HEADER_THREADING_HPP

#include <Dlink/compiler_metadata.hpp>

#include <cstddef>

namespace dlink
{
	struct threading_info final
	{
		std::size_t average = 0;
		std::size_t remainder = 0;
		std::size_t count_of_threads = 0;
	};

	threading_info get_threading_info(const compiler_metadata& metadata);
}

#endif