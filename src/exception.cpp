#include <Dlink/exception.hpp>

namespace dlink
{
	invalid_state::invalid_state(const std::string_view& message)
		: std::runtime_error(message.data())
	{}
}