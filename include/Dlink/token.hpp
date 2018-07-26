#ifndef DLINK_HEADER_TOKEN_HPP
#define DLINK_HEADER_TOKEN_HPP

#include <string>

namespace dlink
{
	enum class token_type
	{
		none,
		eof,

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
	};

	std::string to_string(token_type type);
}

#endif