#ifndef DLINK_HEADER_MESSAGE_HPP
#define DLINK_HEADER_MESSAGE_HPP

#include <Dlink/vector.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include <boost/format.hpp>

namespace dlink
{
	enum class message_type
	{
		info,
		warning,
		error,
	};

	class message
	{
	public:
		message(std::uint16_t id, const std::string_view& what);
		message(std::uint16_t id, const std::string_view& what, const std::string_view& where);
		message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
			const std::string_view& additional_note);
		message(const message& message);
		message(message&& message) noexcept = delete;
		virtual ~message() = default;
		
	public:
		message& operator=(const message& message) = delete;
		message& operator=(message&& message) noexcept = delete;
		bool operator==(const message& message) const = delete;
		bool operator!=(const message& message) const = delete;

	public:
		virtual message_type type() const noexcept = 0;

	public:
		std::string full_id() const;

	public:
		std::uint16_t id() const noexcept;
		const std::string& what() const noexcept;
		const std::string& additional_note() const noexcept;
		const std::string& where() const noexcept;

	private:
		std::uint16_t id_;
		std::string what_;
		std::string additional_note_;
		std::string where_;
	};

	using message_ptr = std::shared_ptr<message>;

	class info_message final : public message
	{
	public:
		info_message(std::uint16_t id, const std::string_view& what);
		info_message(std::uint16_t id, const std::string_view& what, const std::string_view& where);
		info_message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
			const std::string_view& additional_note);
		info_message(const info_message& message);
		info_message(info_message&& message) noexcept = delete;
		virtual ~info_message() override = default;

	public:
		info_message& operator=(const info_message& message) = delete;
		info_message& operator=(info_message&& message) noexcept = delete;
		bool operator==(const info_message& message) const = delete;
		bool operator!=(const info_message& message) const = delete;

	public:
		virtual message_type type() const noexcept override;
	};

	class warning_message final : public message
	{
	public:
		warning_message(std::uint16_t id, const std::string_view& what);
		warning_message(std::uint16_t id, const std::string_view& what, const std::string_view& where);
		warning_message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
			const std::string_view& additional_note);
		warning_message(const info_message& message);
		warning_message(warning_message&& message) noexcept = delete;
		virtual ~warning_message() override = default;

	public:
		warning_message& operator=(const warning_message& message) = delete;
		warning_message& operator=(warning_message&& message) noexcept = delete;
		bool operator==(const warning_message& message) const = delete;
		bool operator!=(const warning_message& message) const = delete;

	public:
		virtual message_type type() const noexcept override;
	};

	class error_message final : public message
	{
	public:
		error_message(std::uint16_t id, const std::string_view& what);
		error_message(std::uint16_t id, const std::string_view& what, const std::string_view& where);
		error_message(std::uint16_t id, const std::string_view& what, const std::string_view& where,
			const std::string_view& additional_note);
		error_message(const error_message& message);
		error_message(error_message&& message) noexcept = delete;
		virtual ~error_message() override = default;

	public:
		error_message& operator=(const error_message& message) = delete;
		error_message& operator=(error_message&& message) noexcept = delete;
		bool operator==(const error_message& message) const = delete;
		bool operator!=(const error_message& message) const = delete;

	public:
		virtual message_type type() const noexcept override;
	};

	class messages final : public vector<message_ptr>
	{
	public:
		bool has_error() const;
		bool has_warning() const;
		bool has_info() const;
	};

	std::string to_string(const message_ptr& message);
	std::string generate_line_col(const std::string_view& path, std::size_t line, std::size_t col);
	std::string generate_source(const std::string_view& source, std::size_t line, std::size_t col, std::size_t length,
								const std::string_view& message = "");

	namespace details
	{
		struct message_formater
		{
			message_formater(const std::string_view& original_message)
				: original_message(original_message)
			{}

			std::string_view operator()() const;
			template<typename... Args_>
			std::string operator()(Args_&&... args) const
			{
				return (boost::format(original_message.data()) % ... % args).str();
			}

			std::string_view original_message;
		};
	}

	class message_data final
	{
	public:
		message_data();
		explicit message_data(const std::string& path);
		message_data(const message_data& data);
		message_data(message_data&& data) noexcept;
		~message_data() = default;

	public:
		message_data& operator=(const message_data& data);
		message_data& operator=(message_data&& data) noexcept;
		bool operator==(const message_data& data) const = delete;
		bool operator!=(const message_data& data) const = delete;

	public:
		void clear() noexcept;
		void swap(message_data& data) noexcept;
		bool empty() const noexcept;

		void load(const std::string& path);

	private:
		void load_english_messages_();

	public:
		details::message_formater error(std::uint16_t id) const;
		details::message_formater warning(std::uint16_t id) const;
		details::message_formater info(std::uint16_t id) const;

	private:
		std::map<std::uint16_t, std::string> error_messages_;
		std::map<std::uint16_t, std::string> warning_messages_;
		std::map<std::uint16_t, std::string> info_messages_;
		
	public:
		static message_data def;
	};
}

#endif