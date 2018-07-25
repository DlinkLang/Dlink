#include <Dlink/compiler_metadata.hpp>
#include <Dlink/compiler_options.hpp>
#include <Dlink/decoder.hpp>
#include <Dlink/message.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>

int main(int argc, char** argv)
{
	dlink::compiler_options options;

	if (!dlink::parse_command_line(argc, argv, options))
	{
		return 0;
	}

	dlink::compiler_metadata metadata(options);

	// Test code

	double avg_multi = 0.0;
	double avg_single = 0.0;

	double max_multi = 0.0;
	double min_multi = 10000;
	double max_single = 0.0;
	double min_single = 10000;

	int C = 20000;
	std::vector<dlink::source> results;

	for (int i = 0; i < C; ++i)
	{
		results.clear();

		auto a = std::chrono::system_clock::now();

		dlink::decoder::decode(metadata, results);

		auto b = std::chrono::system_clock::now();
		std::chrono::duration<double> c = b - a;

		avg_multi += c.count();
		max_multi = std::max(max_multi, c.count());
		min_multi = std::min(min_multi, c.count());
		
		results.clear();

		auto d = std::chrono::system_clock::now();

		dlink::decoder::decode_singlethread(metadata, results);

		auto e = std::chrono::system_clock::now();
		std::chrono::duration<double> f = e - d;

		avg_single += f.count();
		max_single = std::max(max_single, f.count());
		min_single = std::min(min_single, f.count());
	}

	avg_multi /= C;
	avg_single /= C;

	std::cout << C << " times\n\n<Multithread>\nAverage: " << avg_multi << "sec\n";
	std::cout << "Max: " << max_multi << "sec\nMin: " << min_multi << "sec\n";
	std::cout << "\n<Singlethread>\nAverage: " << avg_single << "sec\n";
	std::cout << "Max: " << max_single << "sec\nMin: " << min_single << "sec\n";

	return 0;
}

#include <climits>
#include <cstddef>
#include <cstdint>

static_assert(sizeof(char) == sizeof(std::int8_t));
static_assert(sizeof(std::size_t) >= sizeof(std::int32_t));
static_assert(CHAR_BIT == 8);