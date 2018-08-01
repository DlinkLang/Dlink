#include <Dlink/message.hpp>

#include <algorithm>
#include <stdexcept>

namespace dlink
{
	message::message(std::uint16_t id, const std::string_view& what)
		: id_(id), what_(what)
	{}
	message::message(std::uint16_t id, const std::string_view& what, const std::string_view& where)
		: id_(id), what_(what), where_(where)
	{}
	message::message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
		const std::string_view& additional_note)
		: id_(id), what_(what), where_(where), additional_note_(additional_note)
	{}
	message::message(const message& message)
		: id_(message.id_), what_(message.what_), where_(message.where_),
		additional_note_(message.additional_note_)
	{}
	
	std::string message::full_id() const
	{
		std::string result;
		result.resize(2);

		result[0] = 'D';

		switch (type())
		{
		case message_type::info:
			result[1] = 'I';
			break;

		case message_type::warning:
			result[1] = 'W';
			break;

		case message_type::error:
			result[1] = 'E';
			break;

		default:
			throw std::runtime_error("The result of the method 'dlink::message::type(void) const noexcept' isn't valid.");
		}

		result += std::to_string(id_);

		if (result.size() != 6)
		{
			result.insert(result.begin() + 2, '0');
		}

		return result;
	}

	std::uint16_t message::id() const noexcept
	{
		return id_;
	}
	const std::string& message::what() const noexcept
	{
		return what_;
	}
	const std::string& message::additional_note() const noexcept
	{
		return additional_note_;
	}
	const std::string& message::where() const noexcept
	{
		return where_;
	}
}

namespace dlink
{
	info_message::info_message(std::uint16_t id, const std::string_view& what)
		: message(id, what)
	{}
	info_message::info_message(std::uint16_t id, const std::string_view& what, const std::string_view& where)
		: message(id, what, where)
	{}
	info_message::info_message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
		const std::string_view& additional_note)
		: message(id, what, where, additional_note)
	{}
	info_message::info_message(const info_message& message)
		: message(message)
	{}

	message_type info_message::type() const noexcept
	{
		return message_type::info;
	}
}

namespace dlink
{
	warning_message::warning_message(std::uint16_t id, const std::string_view& what)
		: message(id, what)
	{}
	warning_message::warning_message(std::uint16_t id, const std::string_view& what, const std::string_view& where)
		: message(id, what, where)
	{}
	warning_message::warning_message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
		const std::string_view& additional_note)
		: message(id, what, where, additional_note)
	{}
	warning_message::warning_message(const info_message& message)
		: message(message)
	{}

	message_type warning_message::type() const noexcept
	{
		return message_type::warning;
	}
}

namespace dlink
{
	error_message::error_message(std::uint16_t id, const std::string_view& what)
		: message(id, what)
	{}
	error_message::error_message(std::uint16_t id, const std::string_view& what, const std::string_view& where)
		: message(id, what, where)
	{}
	error_message::error_message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
		const std::string_view& additional_note)
		: message(id, what, where, additional_note)
	{}
	error_message::error_message(const error_message& message)
		: message(message)
	{}

	message_type error_message::type() const noexcept
	{
		return message_type::error;
	}
}

namespace dlink
{
	bool messages::has_error() const
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex());
#endif

		for (const message_ptr& message : data_stl())
		{
			if (message->type() == message_type::error)
			{
				return true;
			}
		}

		return false;
	}
	bool messages::has_warning() const
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex());
#endif

		for (const message_ptr& message : data_stl())
		{
			if (message->type() == message_type::warning)
			{
				return true;
			}
		}

		return false;
	}
	bool messages::has_info() const
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex());
#endif

		for (const message_ptr& message : data_stl())
		{
			if (message->type() == message_type::info)
			{
				return true;
			}
		}

		return false;
	}
}

namespace dlink
{
	std::string to_string(const message_ptr& message)
	{
		std::string result;

		switch (message->type())
		{
		case message_type::info:
			result = "Info[";
			break;

		case message_type::warning:
			result = "Warning[";
			break;

		case message_type::error:
			result = "Error[";
			break;

		default:
			break;
		}

		result += message->full_id() + "]: " + message->what();

		if (!message->where().empty())
		{
			result += "\n --> " + message->where();
		}
		if (!message->additional_note().empty())
		{
			result += '\n' + message->additional_note();
		}

		return result;
	}
	std::string generate_line_col(const std::string_view& path, std::size_t line, std::size_t col)
	{
		return std::string(path.data()) + ':' + std::to_string(line) + ':' + std::to_string(col);
	}
	std::string generate_source(const std::string_view& source, std::size_t line, std::size_t col, std::size_t length,
								const std::string_view& message)
	{
		const std::string line_string = std::to_string(line);
		const std::string empty_line = std::string(line_string.size(), ' ') + " |";
		std::string result = std::string(line_string.size(), ' ') + " |\n" +
							 line_string + " | " + std::string(source.data(), source.length()) + '\n';

		result += empty_line;
		result += std::string(col, ' ') + std::string(length, '^');
		
		if (!message.empty())
		{
			result += message;
		}

		return result;
	}
}