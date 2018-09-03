#include <Dlink/token.hpp>

#include <algorithm>
#include <map>
#include <utility>

namespace dlink
{
#define MAP_TOKEN(token) std::pair<token_type, std::string_view>(token_type:: token , #token)

	namespace
	{
		const std::map<token_type, std::string_view> token_map_ =
		{
			MAP_TOKEN(none),
			MAP_TOKEN(eof),

			MAP_TOKEN(identifier),

			MAP_TOKEN(integer_bin),
			MAP_TOKEN(integer_oct),
			MAP_TOKEN(integer_dec),
			MAP_TOKEN(integer_hex),

			MAP_TOKEN(decimal),

			MAP_TOKEN(character),
			MAP_TOKEN(string),

			MAP_TOKEN(plus),
			MAP_TOKEN(increment),
			MAP_TOKEN(plus_assign),

			MAP_TOKEN(minus),
			MAP_TOKEN(decrement),
			MAP_TOKEN(minus_assign),

			MAP_TOKEN(multiply),
			MAP_TOKEN(multiply_assign),

			MAP_TOKEN(divide),
			MAP_TOKEN(divide_assign),

			MAP_TOKEN(modulo),
			MAP_TOKEN(modulo_assign),

			MAP_TOKEN(exp),
			MAP_TOKEN(exp_assign),

			MAP_TOKEN(assign),
			MAP_TOKEN(equal),
			MAP_TOKEN(equal_not),
			MAP_TOKEN(greater),
			MAP_TOKEN(equal_greater),
			MAP_TOKEN(less),
			MAP_TOKEN(equal_less),

			MAP_TOKEN(logic_and),
			MAP_TOKEN(logic_or),

			MAP_TOKEN(bit_not),
			MAP_TOKEN(bit_and),
			MAP_TOKEN(bit_and_assign),
			MAP_TOKEN(bit_or),
			MAP_TOKEN(bit_or_assign),
			MAP_TOKEN(bit_xor),
			MAP_TOKEN(bit_xor_assign),
			MAP_TOKEN(bit_shift_left),
			MAP_TOKEN(bit_shift_left_assign),
			MAP_TOKEN(bit_shift_right),
			MAP_TOKEN(bit_shift_right_assign),

			MAP_TOKEN(rightwards_arrow),
			MAP_TOKEN(rightwards_double_arrow),

			MAP_TOKEN(brace_left),
			MAP_TOKEN(brace_right),
			MAP_TOKEN(paren_left),
			MAP_TOKEN(paren_right),
			MAP_TOKEN(big_paren_left),
			MAP_TOKEN(big_paren_right),

			MAP_TOKEN(dot),
			MAP_TOKEN(comma),
			MAP_TOKEN(apostrophe),
			MAP_TOKEN(semicolon),
			MAP_TOKEN(colon),

			MAP_TOKEN(exclamation),
			MAP_TOKEN(question),

			MAP_TOKEN(dollar),
			MAP_TOKEN(at),

			MAP_TOKEN(keyword_auto),
			MAP_TOKEN(keyword_void),
			MAP_TOKEN(keyword_bool),
			MAP_TOKEN(keyword_char),
			MAP_TOKEN(keyword_char16),
			MAP_TOKEN(keyword_char32),

			MAP_TOKEN(keyword_i8),
			MAP_TOKEN(keyword_i16),
			MAP_TOKEN(keyword_i32),
			MAP_TOKEN(keyword_i64),
			MAP_TOKEN(keyword_u8),
			MAP_TOKEN(keyword_u16),
			MAP_TOKEN(keyword_u32),
			MAP_TOKEN(keyword_u64),

			MAP_TOKEN(keyword_let),
			MAP_TOKEN(keyword_immut),
			MAP_TOKEN(keyword_mut),
			MAP_TOKEN(keyword_const),

			MAP_TOKEN(keyword_func),
			MAP_TOKEN(keyword_class),
			MAP_TOKEN(keyword_union),
			MAP_TOKEN(keyword_module),
			MAP_TOKEN(keyword_domain),
			MAP_TOKEN(keyword_inline),
			MAP_TOKEN(keyword_enum),
			MAP_TOKEN(keyword_public),
			MAP_TOKEN(keyword_internal),
			MAP_TOKEN(keyword_protected),
			MAP_TOKEN(keyword_private),
			MAP_TOKEN(keyword_use),
			MAP_TOKEN(keyword_as),
			MAP_TOKEN(keyword_default),
			MAP_TOKEN(keyword_macro),
			MAP_TOKEN(keyword_panic),

			MAP_TOKEN(keyword_for),
			MAP_TOKEN(keyword_do),
			MAP_TOKEN(keyword_while),
			MAP_TOKEN(keyword_match),
			MAP_TOKEN(keyword_if),
			MAP_TOKEN(keyword_else),
			MAP_TOKEN(keyword_goto),
			MAP_TOKEN(keyword_break),
			MAP_TOKEN(keyword_continue),
			MAP_TOKEN(keyword_return),

			MAP_TOKEN(keyword_extern),
			MAP_TOKEN(keyword_template),
			MAP_TOKEN(keyword_type),
			MAP_TOKEN(keyword_concept),
			MAP_TOKEN(keyword_unsafe),

			MAP_TOKEN(keyword_virtual),
			MAP_TOKEN(keyword_abstract),
			MAP_TOKEN(keyword_open),
			MAP_TOKEN(keyword_this),
			MAP_TOKEN(keyword_super),
			MAP_TOKEN(keyword_static),

			MAP_TOKEN(keyword_async),
			MAP_TOKEN(keyword_await),

			MAP_TOKEN(keyword_bit),
			MAP_TOKEN(keyword_new),
			MAP_TOKEN(keyword_delete),
			MAP_TOKEN(keyword_nullptr),

			MAP_TOKEN(keyword_static_cast),
			MAP_TOKEN(keyword_dynamic_cast),
			MAP_TOKEN(keyword_const_cast),
			MAP_TOKEN(keyword_reinterpret_cast),
			MAP_TOKEN(keyword_is),
			MAP_TOKEN(keyword_typeid),

			MAP_TOKEN(keyword_static_assert),

			MAP_TOKEN(keyword_true),
			MAP_TOKEN(keyword_false),
		};
	}

#undef MAP_TOKEN

	std::string to_string(token_type type)
	{
		return token_map_.at(type).data();
	}
}

namespace dlink
{
	const std::vector<char> special_characters =
	{
		'~',
		'`',
		'!',
		'@',
		'#',
		'$',
		'%',
		'^',
		'&',
		'*',
		'(',
		')',
		'_',
		'-',
		'+',
		'=',
		'|',
		'\\',
		'{',
		'[',
		'}',
		']',
		':',
		';',
		'"',
		'\'',
		'<',
		',',
		'>',
		'.',
		'?',
		'/',
	};

	bool is_special_character(char character)
	{
		return std::find(special_characters.begin(), special_characters.end(), character) !=
			special_characters.end();
	}
}

namespace dlink
{
	token::token(const std::string_view& data, token_type type, std::size_t line, std::size_t col)
		: data_(data), type_(type), line_(line), col_(col)
	{}
	token::token(const token& token)
		: data_(token.data_), type_(token.type_), line_(token.line_), col_(token.col_)
	{}

	token& token::operator=(const token& token)
	{
		data_ = token.data_;
		type_ = token.type_;
		line_ = token.line_;
		col_ = token.col_;

		return *this;
	}

	void token::clear() noexcept
	{
		data_ = "";
		type_ = token_type::none;
		line_ = 0;
		col_ = 0;
	}
	bool token::empty() const noexcept
	{
		return data_.empty() && type_ == token_type::none &&
			line_ == 0 && col_ == 0;
	}

	std::size_t token::line() const noexcept
	{
		return line_;
	}
	void token::line(std::size_t new_line) noexcept
	{
		line_ = new_line;
	}
	std::size_t token::col() const noexcept
	{
		return col_;
	}
	void token::col(std::size_t new_col) noexcept
	{
		col_ = new_col;
	}
	token_type token::type() const noexcept
	{
		return type_;
	}
	void token::type(token_type new_type) noexcept
	{
		type_ = new_type;
	}
	const std::string_view& token::data() const noexcept
	{
		return data_;
	}
	void token::data(const std::string_view& new_data) noexcept
	{
		data_ = new_data;
	}
}