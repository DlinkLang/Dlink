#ifndef DLINK_HEADER_EXCEPTION_HPP
#define DLINK_HEADER_EXCEPTION_HPP

#include <stdexcept>
#include <string_view>

namespace dlink
{
	class invalid_state : public std::runtime_error
	{
	public:
		explicit invalid_state(const std::string_view& message);
		virtual ~invalid_state() override = default;
	};
}

#endif