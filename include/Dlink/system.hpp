#ifndef DLINK_HEADER_SYSTEM_HPP
#define DLINK_HEADER_SYSTEM_HPP

namespace dlink
{
	enum class endian
	{
		none,
		little_endian,
		big_endian,
	};

	endian get_endian();
}

#endif