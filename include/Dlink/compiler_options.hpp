#ifndef DLINK_HEADER_COMPILER_OPTIONS_HPP
#define DLINK_HEADER_COMPILER_OPTIONS_HPP

#include <cstdint>
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
		compiler_options(const compiler_options& options) = delete;
		compiler_options(compiler_options&& options) noexcept = delete;
		~compiler_options() = default;

	public:
		compiler_options& operator=(const compiler_options& options) = delete;
		compiler_options& operator=(compiler_options&& options) noexcept = delete;
		bool operator==(const compiler_options& options) const = delete;
		bool operator!=(const compiler_options& options) const = delete;
		
	public:
		void clear();

		void add_input(const std::string& string);
		void remove_input(const std::string& string);

	public:
		bool help() const noexcept;
		void help(bool new_help) noexcept;
		bool version() const noexcept;
		void version(bool new_version) noexcept;

		std::int32_t count_of_threads() const noexcept;
		void count_of_threads(std::int32_t new_count_of_threads) noexcept;
		const std::vector<std::string>& input_files() const noexcept;
		const std::string& output_file() const noexcept;
		void output_file(const std::string_view& new_output_file);

	private:
		bool help_ = false;
		bool version_ = false;

		std::int32_t count_of_threads_ = 1;
		std::vector<std::string> input_files_;
		std::string output_file_;

	public:
		static constexpr std::int32_t max_count_of_threads = 128;
	};

	bool parse_command_line(int argc, char** argv, compiler_options& option);
	bool parse_command_line(std::ostream& stream, int argc, char** argv, compiler_options& option);
}

#endif