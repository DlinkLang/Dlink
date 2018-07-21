#include <Dlink/decoder.hpp>

#include <Dlink/encoding.hpp>
#include <Dlink/system.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>

#include <Dlink/extlib/utf8.h>

namespace dlink
{
	decoder::decoder(const compiler_options& options)
		: options_(options)
	{}

	void decoder::clear()
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		messages_.clear();
		results_.clear();
	}
	bool decoder::decode()
	{
#ifdef DLINK_MULTITHREADING
		// TODO
		return true;
#else
		return decode_singlethread();
#endif
	}
	bool decoder::decode_singlethread()
	{
		clear();

#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(mutex_);
#endif

		for (const std::string& path : options_.input_files())
		{
			using namespace std::literals::string_literals;

			std::ifstream stream(path);

			if (!stream.is_open())
			{
				messages_.push_back(std::make_shared<error_message>(
					1000, "Failed to open the input.", path
					));
				return false;
			}

			encoding detected_encoding = detect_encoding(stream);

			if (options_.input_encoding() != encoding::none &&
				options_.input_encoding() != detected_encoding)
			{
				messages_.push_back(std::make_shared<error_message>(
					1002, "The input isn't encoded in '"s + to_string(options_.input_encoding()).data() + "'.", path
					));

				return false;
			}

			const std::streampos pos = stream.tellg();
			stream.seekg(0, std::ios::end);

			const std::fpos_t length = stream.tellg().seekpos() - pos.seekpos();
			stream.seekg(pos, std::ios::beg);

			switch (detected_encoding)
			{
			case encoding::none:
			case encoding::utf8:
			{
				std::string str((std::istreambuf_iterator<char>(stream)),
								 std::istreambuf_iterator<char>());

				std::string::iterator invalid_iter = utf8::find_invalid(str.begin(), str.end());

				if (invalid_iter != str.end())
				{
					messages_.push_back(std::make_shared<error_message>(
						1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'.", path
						));

					return false;
				}

				results_.push_back(std::move(str));
				break;
			}

			case encoding::utf16:
			{
				if (length % 2 != 0)
				{
					messages_.push_back(std::make_shared<error_message>(
						1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'.", path
						));

					return false;
				}

				std::u16string temp(static_cast<std::size_t>(length / 2), 0);
				stream.read(reinterpret_cast<char*>(temp.data()), temp.size() * 2);

				if (get_endian() == endian::big_endian)
				{
					std::transform(temp.begin(), temp.end(), temp.begin(), [](char16_t c)
					{
						return static_cast<char16_t>(((c & 0xFF) << 8) + ((c & 0xFF00) >> 8));
					});
				}				

				std::string utf8;
				utf8::utf16to8(temp.begin(), temp.end(), std::back_inserter(utf8));

				results_.push_back(std::move(utf8));
				break;
			}

			case encoding::utf16be:
			{
				if (length % 2 != 0)
				{
					messages_.push_back(std::make_shared<error_message>(
						1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'.", path
						));

					return false;
				}

				std::u16string temp(static_cast<std::size_t>(length / 2), 0);
				stream.read(reinterpret_cast<char*>(temp.data()), temp.size() * 2);

				if (get_endian() == endian::little_endian)
				{
					std::transform(temp.begin(), temp.end(), temp.begin(), [](char16_t c)
					{
						return static_cast<char16_t>(((c & 0xFF) << 8) + ((c & 0xFF00) >> 8));
					});
				}

				std::string utf8;
				utf8::utf16to8(temp.begin(), temp.end(), std::back_inserter(utf8));

				results_.push_back(std::move(utf8));
				break;
			}
			
			case encoding::utf32:
			{
				if (length % 4 != 0)
				{
					messages_.push_back(std::make_shared<error_message>(
						1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'.", path
						));

					return false;
				}

				std::u32string temp(static_cast<std::size_t>(length / 4), 0);
				stream.read(reinterpret_cast<char*>(temp.data()), temp.size() * 4);

				if (get_endian() == endian::big_endian)
				{
					std::transform(temp.begin(), temp.end(), temp.begin(), [](char32_t c)
					{
						return static_cast<char32_t>(((c & 0xFF000000) >> 24) + ((c & 0xFF0000) >> 8) + ((c & 0xFF00) << 8) + ((c & 0xFF) << 24));
					});
				}

				std::string utf8;
				utf8::utf32to8(temp.begin(), temp.end(), std::back_inserter(utf8));

				results_.push_back(std::move(utf8));
				break;
			}
			
			case encoding::utf32be:
			{
				if (length % 4 != 0)
				{
					messages_.push_back(std::make_shared<error_message>(
						1001, "Failed to decode the input using '"s + to_string(detected_encoding).data() + "'.", path
						));

					return false;
				}

				std::u32string temp(static_cast<std::size_t>(length / 4), 0);
				stream.read(reinterpret_cast<char*>(temp.data()), temp.size() * 4);

				if (get_endian() == endian::little_endian)
				{
					std::transform(temp.begin(), temp.end(), temp.begin(), [](char32_t c)
					{
						return static_cast<char32_t>(((c & 0xFF000000) >> 24) + ((c & 0xFF0000) >> 8) + ((c & 0xFF00) << 8) + ((c & 0xFF) << 24));
					});
				}

				std::string utf8;
				utf8::utf32to8(temp.begin(), temp.end(), std::back_inserter(utf8));

				results_.push_back(std::move(utf8));
				break;
			}
			}

			stream.close();
		}

		return true;
	}

	const compiler_options& decoder::options() const noexcept
	{
		return options_;
	}

	const std::vector<message_ptr>& decoder::messages() const noexcept
	{
		return messages_;
	}
	const std::vector<std::string>& decoder::results() const noexcept
	{
		return results_;
	}
}