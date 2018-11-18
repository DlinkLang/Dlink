#ifndef DLINK_HEADER_COMPILER_OPTIONS_HPP
#define DLINK_HEADER_COMPILER_OPTIONS_HPP

#include <Dlink/encoding.hpp>

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#define DLINK_MAJOR (1)
#define DLINK_MINOR (0)
#define DLINK_PATCH (0)
#define DLINK_VERSION ("1.0.0")

namespace dlink
{
	namespace program
	{
		static constexpr int major = DLINK_MAJOR;
		static constexpr int minor = DLINK_MINOR;
		static constexpr int patchlevel = DLINK_PATCH;
		static constexpr const char* version = DLINK_VERSION;
	}

	class compiler_options final
	{
	public:
		compiler_options() = default;
		compiler_options(const compiler_options& options);
		compiler_options(compiler_options&& options) noexcept;
		~compiler_options() = default;

	public:
		compiler_options& operator=(const compiler_options& options);
		compiler_options& operator=(compiler_options&& options) noexcept;
		bool operator==(const compiler_options& options) const = delete;
		bool operator!=(const compiler_options& options) const = delete;
		
	public:
		void clear() noexcept;

		void add_input(const std::string& string);
		void remove_input(const std::string& string);
		void add_macro(const std::string& macro);
		void add_macro(const std::string& macro, const std::string& replaced);

	private:
		void moved_() noexcept;

	public:
		bool help() const noexcept;
		void help(bool new_help) noexcept;
		bool version() const noexcept;
		void version(bool new_version) noexcept;

		std::int32_t count_of_threads() const noexcept;
#ifdef DLINK_MULTITHREADING
		void count_of_threads(std::int32_t new_count_of_threads) noexcept;
#endif
		const std::vector<std::string>& input_files() const noexcept;
		const std::string& output_file() const noexcept;
		void output_file(const std::string_view& new_output_file);

		const std::map<std::string, std::string>& macros() const noexcept;

		encoding input_encoding() const noexcept;
		void input_encoding(encoding new_encoding_type) noexcept;

	private:
		bool help_ = false;
		bool version_ = false;

#ifdef DLINK_MULTITHREADING
		std::int32_t count_of_threads_ = 1;
#endif
		std::vector<std::string> input_files_;
		std::string output_file_;

		std::map<std::string, std::string> macros_;

		encoding input_encoding_ = encoding::none;

	public:
		static constexpr std::int32_t max_count_of_threads = 128;
	};
	
	bool parse_command_line(int argc, char** argv, compiler_options& options);
	bool parse_command_line(std::ostream& stream, int argc, char** argv, compiler_options& options);
}

#endif