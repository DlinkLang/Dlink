#include <Dlink/decoder.hpp>

#include <Dlink/encoding.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>

#ifdef DLINK_MULTITHREADING
#	include <future>
#	include <mutex>
#	include <thread>
#endif

#include <Dlink/extlib/utf8.h>

namespace dlink
{
	bool decoder::decode(compiler_metadata& metadata, std::vector<source>& results)
	{
#ifdef DLINK_MULTITHREADING
		const std::size_t results_size = results.size();
		std::size_t count_of_threads = metadata.options().count_of_threads();

		if (count_of_threads == 0)
		{
			count_of_threads = std::thread::hardware_concurrency();
		
			if (count_of_threads == 0)
			{
				count_of_threads = 4;
			}
		}

		count_of_threads = std::min(count_of_threads, metadata.options().input_files().size());

		if (count_of_threads == 1)
		{
			return decode_singlethread(metadata, results);
		}

		const std::size_t input_files_size = metadata.options().input_files().size();

		std::size_t average = input_files_size / count_of_threads;
		std::size_t remainder = input_files_size % count_of_threads;

		if (average < 4)
		{
			count_of_threads = input_files_size / 4;

			average = input_files_size / count_of_threads;
			remainder = input_files_size % count_of_threads;
		}

		if (remainder != 0)
		{
			const std::size_t remainder_average = remainder / count_of_threads;
			const std::size_t remainder_remainder = remainder % count_of_threads;

			average += remainder_average;
			remainder = remainder_remainder;
		}

		static auto decode_multithread = [&](std::size_t begin, std::size_t end) -> bool
		{
			bool result = true;

			for (std::size_t i = begin; i < end; ++i)
			{
				result = result && decode_source(results[i], metadata);
			}

			return result;
		};

		for (std::size_t i = 0; i < input_files_size; ++i)
		{
			results.emplace_back(metadata.options().input_files()[i]);
		}

		std::vector<std::future<bool>> futures;

		for (std::size_t i = 0; i < count_of_threads - 1; ++i)
		{
			futures.push_back(
				std::async(decode_multithread, i * average + results_size, (i + 1) * average + results_size)
			);
		}

		futures.push_back(
			std::async(decode_multithread, (count_of_threads - 1) * average + results_size, count_of_threads * average + results_size + remainder)
		);

		bool result = true;

		for (std::future<bool>& future : futures)
		{
			future.wait();
			result = result && future.get();
		}

		return result;
#else
		return decode_singlethread(metadata, results);
#endif
	}
	std::pair<bool, std::vector<source>> decoder::decode(compiler_metadata& metadata)
	{
		std::pair<bool, std::vector<source>> result;
		result.first = decode(metadata, result.second);

		return result;
	}
	bool decoder::decode_singlethread(compiler_metadata& metadata, std::vector<source>& results)
	{
		const std::size_t size = results.size();

		for (const std::string& path : metadata.options().input_files())
		{
			source& src = results.emplace_back(path);
			bool result = decode_source(src, metadata);

			if (!result)
			{
				results.erase(results.begin() + size, results.end() - 1);
				return false;
			}
		}

		return true;
	}
	std::pair<bool, std::vector<source>> decoder::decode_singlethread(compiler_metadata& metadata)
	{
		std::pair<bool, std::vector<source>> result;
		result.first = decode_singlethread(metadata, result.second);

		return result;
	}
	bool decoder::decode_source(source& source, compiler_metadata& metadata)
	{
		using namespace std::string_literals;

		std::ifstream stream(source.path());

		if (!stream.is_open())
		{
			metadata.messages().push_back(std::make_shared<error_message>(
				1000, "Failed to open the input.", source.path()
				));

			return false;
		}

		encoding detected_encoding = detect_encoding(stream);

		if (metadata.options().input_encoding() != encoding::none &&
			metadata.options().input_encoding() != detected_encoding)
		{
			metadata.messages().push_back(std::make_shared<error_message>(
				1002, "The input isn't encoded in '"s + to_string(metadata.options().input_encoding()).data() + "'.", source.path()
				));

			return false;
		}

		const std::streampos pos = stream.tellg();
		stream.seekg(0, std::ios::end);

		const std::fpos_t length = stream.tellg().seekpos() - pos.seekpos();
		stream.seekg(pos, std::ios::beg);

		switch (detected_encoding)
		{
		case encoding::utf16:
			return decode_utf16_(endian::little_endian, length, detected_encoding, stream,
								 source, metadata);
			
		case encoding::utf16be:
			return decode_utf16_(endian::big_endian, length, detected_encoding, stream,
				source, metadata);

		case encoding::utf32:
			return decode_utf32_(endian::little_endian, length, detected_encoding, stream,
				source, metadata);
			
		case encoding::utf32be:
			return decode_utf32_(endian::big_endian, length, detected_encoding, stream,
				source, metadata);

		default:
		{
			std::string str((std::istreambuf_iterator<char>(stream)),
				std::istreambuf_iterator<char>());
			std::string::iterator invalid_iter = utf8::find_invalid(str.begin(), str.end());

			if (invalid_iter != str.end())
			{
				metadata.messages().push_back(std::make_shared<error_message>(
					1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'.", source.path()
					));

				return false;
			}

			source.codes(std::move(str));
			return true;
		}
		}
	}

	bool decoder::decode_utf16_(const endian encoding_endian, const std::fpos_t length, const encoding detected_encoding,
								std::ifstream& stream, source& source, compiler_metadata& metadata)
	{
		using namespace std::string_literals;

		if (length % 2 != 0)
		{
			metadata.messages().push_back(std::make_shared<error_message>(
				1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'."
				));

			return false;
		}

		std::u16string temp(static_cast<std::size_t>(length / 2), 0);
		stream.read(reinterpret_cast<char*>(temp.data()), temp.size() * 2);

		const endian system_endian = get_endian();

		if ((system_endian == endian::big_endian && encoding_endian == endian::little_endian) ||
			(system_endian == endian::little_endian && encoding_endian == endian::big_endian))
		{
			std::transform(temp.begin(), temp.end(), temp.begin(), [](char16_t c)
			{
				return static_cast<char16_t>(((c & 0xFF) << 8) + ((c & 0xFF00) >> 8));
			});
		}

		std::string utf8;
		utf8::utf16to8(temp.begin(), temp.end(), std::back_inserter(utf8));

		source.codes(std::move(utf8));

		return true;
	}
	bool decoder::decode_utf32_(const endian encoding_endian, const std::fpos_t length, const encoding detected_encoding,
								std::ifstream& stream, source& source, compiler_metadata& metadata)
	{
		using namespace std::string_literals;

		if (length % 4 != 0)
		{
			metadata.messages().push_back(std::make_shared<error_message>(
				1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'."
				));

			return false;
		}

		std::u32string temp(static_cast<std::size_t>(length / 4), 0);
		stream.read(reinterpret_cast<char*>(temp.data()), temp.size() * 4);

		const endian system_endian = get_endian();

		if ((system_endian == endian::big_endian && encoding_endian == endian::little_endian) ||
			(system_endian == endian::little_endian && encoding_endian == endian::big_endian))
		{
			std::transform(temp.begin(), temp.end(), temp.begin(), [](char32_t c)
			{
				return static_cast<char32_t>(((c & 0xFF000000) >> 24) + ((c & 0xFF0000) >> 8) + ((c & 0xFF00) << 8) + ((c & 0xFF) << 24));
			});
		}

		std::string utf8;
		utf8::utf32to8(temp.begin(), temp.end(), std::back_inserter(utf8));

		source.codes(std::move(utf8));

		return true;
	}
}