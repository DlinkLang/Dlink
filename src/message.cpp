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

		return result + std::to_string(id_);
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
	const message_ptr& messages::operator[](std::size_t index) const
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_[index];
	}
	message_ptr& messages::operator[](std::size_t index)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_[index];
	}

	const message_ptr& messages::at(std::size_t index) const
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.at(index);
	}
	message_ptr& messages::at(std::size_t index)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.at(index);
	}
	messages::const_iterator messages::begin() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.begin();
	}
	messages::iterator messages::begin() noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.begin();
	}
	messages::const_iterator messages::cbegin() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.cbegin();
	}
	messages::const_iterator messages::end() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.end();
	}
	messages::iterator messages::end() noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.end();
	}
	messages::const_iterator messages::cend() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.cend();
	}
	messages::const_reverse_iterator messages::rbegin() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.rbegin();
	}
	messages::reverse_iterator messages::rbegin() noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.rbegin();
	}
	messages::const_reverse_iterator messages::crbegin() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.crbegin();
	}
	messages::const_reverse_iterator messages::rend() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.rend();
	}
	messages::reverse_iterator messages::rend() noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.rend();
	}
	messages::const_reverse_iterator messages::crend() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.crend();
	}

	bool messages::empty() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.empty();
	}
	std::size_t messages::size() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.size();
	}
	std::size_t messages::capacity() const noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.capacity();
	}
	void messages::resize(std::size_t count)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.resize(count);
	}
	void messages::resize(std::size_t count, const message_ptr& message)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.resize(count, message);
	}
	void messages::shrink_to_fit()
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.shrink_to_fit();
	}
	void messages::reserve(std::size_t new_capacity)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.reserve(new_capacity);
	}
	void messages::clear() noexcept
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.clear();
	}

	void messages::push_back(const message_ptr& message)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.push_back(message);
	}
	void messages::push_back(message_ptr&& message)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.push_back(std::move(message));
	}
	void messages::pop_back()
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.pop_back();
	}
	messages::iterator messages::insert(const_iterator iterator, const message_ptr& message)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.insert(iterator, message);
	}
	messages::iterator messages::insert(const_iterator iterator, message_ptr&& message)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.insert(iterator, std::move(message));
	}
	messages::iterator messages::insert(const_iterator iterator, std::size_t count, const message_ptr& message)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.insert(iterator, count, message);
	}
	messages::iterator messages::insert(const_iterator iterator, std::initializer_list<message_ptr> list)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.insert(iterator, list);
	}
	messages::iterator messages::erase(const_iterator iterator)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.erase(iterator);
	}
	messages::iterator messages::erase(const_iterator first, const_iterator last)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		return messages_.erase(first, last);
	}

	bool messages::has_error() const
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		for (const message_ptr& message : messages_)
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
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		for (const message_ptr& message : messages_)
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
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		for (const message_ptr& message : messages_)
		{
			if (message->type() == message_type::info)
			{
				return true;
			}
		}

		return false;
	}

	const std::vector<message_ptr>& messages::data() const noexcept
	{
		return messages_;
	}
	std::vector<message_ptr>& messages::data() noexcept
	{
		return messages_;
	}
}