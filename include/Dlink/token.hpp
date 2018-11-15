#ifndef DLINK_HEADER_TOKEN_HPP
#define DLINK_HEADER_TOKEN_HPP

#include <Dlink/vector.hpp>
#include <Dlink/extlib/json.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace dlink
{
	enum class token_type
	{
		none,
		none_sc,				// Special character
		none_hm,				// Has meaning
		whitespace,

		identifier,

		integer_bin,
		integer_oct,
		integer_dec,
		integer_hex,

		decimal,

		character,				// '~'
		string,					// "~~~"

		plus,					// +
		increment,				// ++
		plus_assign,			// +=

		minus,					// -
		decrement,				// --
		minus_assign,			// -=

		multiply,				// *
		multiply_assign,		// *=

		divide,					// /
		divide_assign,			// /=

		modulo,					// %
		modulo_assign,			// %=

		exp,					// **
		exp_assign,				// **=

		assign,					// =
		equal,					// ==
		equal_not,				// !=
		greater,				// >
		equal_greater,			// >=
		less,					// <
		equal_less,				// <=

		logic_and,				// &&
		logic_or,				// ||

		bit_not,				// ~
		bit_and,				// &
		bit_and_assign,			// &=
		bit_or,					// |
		bit_or_assign,			// |=
		bit_xor,				// ^
		bit_xor_assign,			// ^=
		bit_shift_left,			// <<
		bit_shift_left_assign,	// <<=
		bit_shift_right,		// >>
		bit_shift_right_assign, // >>=

		rightwards_arrow,		// ->
		rightwards_double_arrow,// =>

		brace_left,				// {
		brace_right,			// }
		paren_left,				// (
		paren_right,			// )
		big_paren_left,			// [
		big_paren_right,		// ]

		dot,					// .
		comma,					// ,
		apostrophe,				// '
		semicolon,				// ;
		colon,					// :

		exclamation,			// !
		question,				// ?

		dollar,					// $
		at,						// @

		keyword_auto,
		keyword_void,
		keyword_bool,
		keyword_char,
		keyword_char16,
		keyword_char32,

		keyword_i8,
		keyword_i16,
		keyword_i32,
		keyword_i64,
		keyword_u8,
		keyword_u16,
		keyword_u32,
		keyword_u64,

		keyword_let,
		keyword_immut,
		keyword_mut,
		keyword_const,

		keyword_func,
		keyword_class,
		keyword_union,
		keyword_module,
		keyword_domain,
		keyword_inline,
		keyword_enum,
		keyword_public,
		keyword_internal,
		keyword_protected,
		keyword_private,
		keyword_use,
		keyword_as,
		keyword_default,
		keyword_macro,
		keyword_panic,

		keyword_for,
		keyword_do,
		keyword_while,
		keyword_match,
		keyword_if,
		keyword_else,
		keyword_goto,
		keyword_break,
		keyword_continue,
		keyword_return,

		keyword_extern,
		keyword_template,
		keyword_type,
		keyword_concept,
		keyword_unsafe,

		keyword_virtual,
		keyword_abstract,
		keyword_open,
		keyword_this,
		keyword_super,
		keyword_static,

		keyword_async,
		keyword_await,

		keyword_bit,
		keyword_new,
		keyword_delete,
		keyword_nullptr,

		keyword_static_cast,
		keyword_dynamic_cast,
		keyword_const_cast,
		keyword_reinterpret_cast,
		keyword_is,
		keyword_typeid,

		keyword_static_assert,

		keyword_true,
		keyword_false,
	};

	std::string to_string(token_type type);

	extern const std::vector<char> special_characters;
	bool is_special_character(char character);
	bool is_valid_special_character(char character) noexcept;
	token_type to_token_type(char valid_special_character) noexcept;
	bool is_single_special_character(char valid_special_character) noexcept;
	token_type complex_token_type(token_type type, char valid_special_character) noexcept;
	
	class token final
	{
	public:
		token() = default;
		token(const std::string_view& data, token_type type, std::size_t line, std::size_t col, const std::string_view& line_data);
		token(const std::string_view& data, token_type type, std::size_t line, std::size_t col, const std::string_view& line_data,
			const std::string_view& prefix_literal, const std::string_view& postfix_literal);
		token(const token& token);
		~token() = default;

	public:
		token& operator=(const token& token);
		bool operator==(const token& token) const = delete;
		bool operator!=(const token& token) const = delete;

	public:
		void clear() noexcept;
		bool empty() const noexcept;

		nlohmann::json dump() const;

	public:
		std::size_t line() const noexcept;
		void line(std::size_t new_line) noexcept;
		std::size_t col() const noexcept;
		void col(std::size_t new_col) noexcept;
		token_type type() const noexcept;
		void type(token_type new_type) noexcept;
		const std::string_view& data() const noexcept;
		void data(const std::string_view& new_data) noexcept;
		const std::string_view& line_data() const noexcept;
		void line_data(const std::string_view& new_line_data) noexcept;
		const std::string_view& prefix_literal() const noexcept;
		void prefix_literal(const std::string_view& new_prefix_literal) noexcept;
		const std::string_view& postfix_literal() const noexcept;
		void postfix_literal(const std::string_view& new_postfix_literal) noexcept;

	private:
		std::size_t line_ = 0;
		std::size_t col_ = 0;
		token_type type_ = token_type::none;
		std::string_view data_;
		std::string_view line_data_;
		std::string_view prefix_literal_;
		std::string_view postfix_literal_;
	};

	using tokens = vector<token>;
}

#endif