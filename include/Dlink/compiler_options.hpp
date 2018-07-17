#ifndef DLINK_HEADER_COMPILER_OPTIONS_HPP
#define DLINK_HEADER_COMPILER_OPTIONS_HPP

#include <cstdint>
#include <ostream>

#ifdef DLINK_MULTITHREADING
#	include <atomic>
#endif

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
		using int32_t =
#ifdef DLINK_MULTITHREADING
			std::atomic_int32_t;
#else
			std::int32_t;
#endif
		using bool_t =
#ifdef DLINK_MULTITHREADING
			std::atomic_bool;
#else
			bool;
#endif

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

	public:
		bool help() const noexcept;
		void help(bool new_help) noexcept;
		bool version() const noexcept;
		void version(bool new_version) noexcept;
		std::int32_t count_of_threads() const noexcept;
		void count_of_threads(std::int32_t new_count_of_threads) noexcept;

	private:
		bool_t help_ = false;
		bool_t version_ = false;
		int32_t count_of_threads_ = -1;
	};
}

#endif