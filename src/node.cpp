#include <Dlink/ast/node.hpp>

namespace dlink::ast
{
	node::node(const dlink::token& token)
		: token_(token)
	{}

	token node::token() const noexcept
	{
		return token_;
	}
}