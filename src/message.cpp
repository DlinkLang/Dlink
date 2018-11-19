#include <Dlink/message.hpp>

#include <Dlink/encoding.hpp>
#include <Dlink/extlib/json.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <utility>

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
		const auto [replaced_source, replaced_pos] = replace_with_space(source);
		const std::string empty_line = std::string(line_string.size(), ' ') + " |";
		std::string result = std::string(line_string.size(), ' ') + " |\n" +
							 line_string + " | " + replaced_source + '\n';

		std::size_t col_plus = 0;
		std::size_t len_plus = 0;

		for (const std::pair<std::size_t, std::size_t>& pos : replaced_pos)
		{
			const std::size_t pos_col = pos.first + 1;
			const std::size_t plus = pos.second - 1;

			if (pos_col >= col && pos_col < col + length)
			{
				len_plus += plus;
			}
			else if (pos_col < col)
			{
				col_plus += plus;
			}
		}

		result += empty_line;
		result += std::string(col + col_plus, ' ') + std::string(length + len_plus, '^');
		
		if (!message.empty())
		{
			result += message;
		}

		return result;
	}
}

namespace dlink::details
{
	std::string_view message_formater::operator()() const
	{
		return original_message;
	}
}

namespace dlink
{
	message_data::message_data()
	{
		load_english_messages_();
	}
	message_data::message_data(const std::string& path)
	{
		load(path);
	}
	message_data::message_data(const message_data& data)
		: error_messages_(data.error_messages_), warning_messages_(data.warning_messages_), info_messages_(data.info_messages_)
	{}
	message_data::message_data(message_data&& data) noexcept
		: error_messages_(std::move(data.error_messages_)), warning_messages_(std::move(data.warning_messages_)), info_messages_(std::move(data.info_messages_))
	{}

	message_data& message_data::operator=(const message_data& data)
	{
		error_messages_ = data.error_messages_;
		warning_messages_ = data.warning_messages_;
		info_messages_ = data.info_messages_;

		return *this;
	}
	message_data& message_data::operator=(message_data&& data) noexcept
	{
		error_messages_ = std::move(data.error_messages_);
		warning_messages_ = std::move(data.warning_messages_);
		info_messages_ = std::move(data.info_messages_);

		return *this;
	}

	void message_data::clear() noexcept
	{
		error_messages_.clear();
		warning_messages_.clear();
		info_messages_.clear();
	}
	void message_data::swap(message_data& data) noexcept
	{
		error_messages_.swap(data.error_messages_);
		warning_messages_.swap(data.warning_messages_);
		info_messages_.swap(data.info_messages_);
	}
	bool message_data::empty() const noexcept
	{
		return error_messages_.empty() && warning_messages_.empty() && info_messages_.empty();
	}

	void message_data::load(const std::string& path)
	{
		if (path.empty())
			throw std::invalid_argument("Argument 'path' can't be empty.");

		std::ifstream stream(path);
		if (!stream.is_open())
			throw std::runtime_error("Failed to open the language file.");

		nlohmann::json object;
		stream >> object;

		stream.close();
		load_english_messages_();
		
		if (object.find("error") != object.end())
		{
			nlohmann::json& error = object["error"];

			for (nlohmann::json::iterator message = error.begin(); message != error.end(); ++message)
			{
				error_messages_[static_cast<std::uint16_t>(std::stoi(message.key()))] = message.value();
			}
		}
		if (object.find("warning") != object.end())
		{
			nlohmann::json& warning = object["warning"];

			for (nlohmann::json::iterator message = warning.begin(); message != warning.end(); ++message)
			{
				warning_messages_[static_cast<std::uint16_t>(std::stoi(message.key()))] = message.value();
			}
		}
		if (object.find("info") != object.end())
		{
			nlohmann::json& info = object["info"];

			for (nlohmann::json::iterator message = info.begin(); message != info.end(); ++message)
			{
				info_messages_[static_cast<std::uint16_t>(std::stoi(message.key()))] = message.value();
			}
		}
	}

	void message_data::load_english_messages_()
	{
		error_messages_[1000] = "Failed to open the input.";
		error_messages_[1001] = "Failed to decode the input using '%1%'.";
		error_messages_[1002] = "The input isn't encoded in '%1%'.";

		error_messages_[1100] = "Unexpected EOF found in preprocessor directive.";
		error_messages_[1101] = "Unexpected token found in preprocessor directive name.";
		error_messages_[1103] = "Occurred due to #error.";
		error_messages_[1104] = "#error: %1%";
		error_messages_[1105] = "Unknown preprocessor directive.";

		error_messages_[2000] = "Invalid digit '%1%' in binary literal.";
		error_messages_[2001] = "Invalid digit '%1%' in octal literal.";
		error_messages_[2003] = "Invalid digit '%1%' in hexadecimal literal.";
		error_messages_[2004] = "Invalid binary literal.";
		error_messages_[2005] = "Invalid hexadecimal literal.";
		error_messages_[2006] = "'%1%' is an invalid token.";
		error_messages_[2007] = "Unexpected EOF found in comment.";
		error_messages_[2008] = "Unexpected EOL found in character literal.";
		error_messages_[2009] = "Unexpected EOL found in string literal.";
		error_messages_[2010] = "Invalid scientific notation format.";
		error_messages_[2011] = "Invalid decimal literal format.";

		warning_messages_[1100] = "Occurred due to #warning.";
		warning_messages_[1101] = "#warning: %1%";
	}

	details::message_formater message_data::error(std::uint16_t id) const
	{
		return std::string_view(error_messages_.at(id));
	}
	details::message_formater message_data::warning(std::uint16_t id) const
	{
		return std::string_view(warning_messages_.at(id));
	}
	details::message_formater message_data::info(std::uint16_t id) const
	{
		return std::string_view(info_messages_.at(id));
	}

	message_data message_data::def;
}