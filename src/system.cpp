#include <Dlink/system.hpp>

#include <cstdint>
#include <stdexcept>

namespace dlink
{
	dlink::endian get_endian()
	{
		static dlink::endian endian = dlink::endian::none;

		if (endian == dlink::endian::none)
		{
			std::uint32_t i = 1;

			if (*reinterpret_cast<std::uint8_t*>(&i) == 1)
			{
				endian = dlink::endian::little_endian;
			}
			else if (*(reinterpret_cast<std::uint8_t*>(&i) + sizeof(std::uint32_t) - 1) == 1)
			{
				endian = dlink::endian::big_endian;
			}
			else
				throw std::runtime_error("Failed to detect endianness.");
		}

		return endian;
	}
}