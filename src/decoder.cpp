#include <Dlink/decoder.hpp>

#include <Dlink/encoding.hpp>
#include <Dlink/exception.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>

#ifdef DLINK_MULTITHREADING
#	include <Dlink/threading.hpp>

#	include <future>
#	include <mutex>
#endif

#include <Dlink/extlib/utf8/utf8.h>

namespace dlink
{
	bool decoder::decode(compiler_metadata& metadata, std::vector<source>& results)
	{
#ifdef DLINK_MULTITHREADING
		auto decode_multithread = [&](std::size_t begin, std::size_t end) -> bool
		{
			bool result = true;

			for (std::size_t i = begin; i < end; ++i)
			{
				result = result && decode_source(results[i], metadata);
			}

			return result;
		};

		for (const std::string& path : metadata.options().input_files())
		{
			results.emplace_back(path);
		}

		return parallel(decode_multithread, get_threading_info(metadata), results.size());
#else
		return decode_singlethread(metadata, results);
#endif
	}
	bool decoder::decode_singlethread(compiler_metadata& metadata, std::vector<source>& results)
	{
		const std::size_t size = results.size();
		bool result = true;

		for (const std::string& path : metadata.options().input_files())
		{
			source& src = results.emplace_back(path);
			result = result && decode_source(src, metadata);
		}

		return result;
	}
	bool decoder::decode_source(source& source, compiler_metadata& metadata)
	{
		if (source.state() != source_state::initialized)
			throw invalid_state("The state of the argument 'source' must be 'dlink::source_state::initialized' when 'static bool dlink::decoder::decode_source(dlink::source&, dlink::compiler_metadata&)' method is called.");

		std::ifstream stream(source.path(), std::ios::binary);

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
				1002, "The input isn't encoded in '" + to_string(metadata.options().input_encoding()) + "'.", source.path()
				));

			return false;
		}

		const std::streampos pos = stream.tellg();
		stream.seekg(0, std::ios::end);

		const std::streamoff length = stream.tellg() - pos;
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
					1001, "Failed to decode the input using '" + to_string(detected_encoding) + "'.", source.path()
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
		if (length % 2 != 0)
		{
			metadata.messages().push_back(std::make_shared<error_message>(
				1001, "Failed to decode the input using '" + to_string(detected_encoding) + "'."
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
		if (length % 4 != 0)
		{
			metadata.messages().push_back(std::make_shared<error_message>(
				1001, "Failed to decode the input using '" + to_string(detected_encoding) + "'."
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