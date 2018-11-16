#ifndef DLINK_HEADER_AST_NODE_HPP
#define DLINK_HEADER_AST_NODE_HPP

#include <Dlink/token.hpp>
#include <Dlink/extlib/json.hpp>

#include <memory>

namespace dlink::ast
{
	class node
	{
	public:
		explicit node(const token& token);
		node(const node& node) = delete;
		node(node&& node) = delete;
		virtual ~node() = default;

	public:
		node& operator=(const node& node) = delete;
		node& operator=(node&& node) noexcept = delete;
		bool operator==(const node& node) const = delete;
		bool operator!=(const node& node) const = delete;

	public:
		virtual nlohmann::json dump() const = 0;

	public:
		token token() const noexcept;

	private:
		const dlink::token token_;
	};

	class expression : public node
	{
	public:
		using node::node;
		virtual ~expression() override = default;

	public:
		expression& operator=(const expression& node) = delete;
		expression& operator=(expression&& node) noexcept = delete;
		bool operator==(const expression& node) const = delete;
		bool operator!=(const expression& node) const = delete;
	};

	class statement : public node
	{
	public:
		using node::node;
		virtual ~statement() override = default;

	public:
		statement& operator=(const statement& node) = delete;
		statement& operator=(statement&& node) noexcept = delete;
		bool operator==(const statement& node) const = delete;
		bool operator!=(const statement& node) const = delete;
	};
}

#endif