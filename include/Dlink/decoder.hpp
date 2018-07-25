#ifndef DLINK_HEADER_DECODER_HPP
#define DLINK_HEADER_DECODER_HPP

#include <Dlink/compiler_metadata.hpp>
#include <Dlink/source.hpp>
#include <Dlink/system.hpp>

#include <cstdio>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace dlink
{
	class decoder final
	{
	public:
		decoder() = delete;
		decoder(const decoder& decoder) = delete;
		decoder(decoder&& decoder) noexcept = delete;
		~decoder() = default;

	public:
		decoder& operator=(const decoder& decoder) = delete;
		decoder& operator=(decoder&& decoder) noexcept = delete;
		bool operator==(const decoder& decoder) const = delete;
		bool operator!=(const decoder& decoder) const = delete;

	public:
		static bool decode(compiler_metadata& metadata, std::vector<source>& results);
		static std::pair<bool, std::vector<source>> decode(compiler_metadata& metadata);
		static bool decode_singlethread(compiler_metadata& metadata, std::vector<source>& results);
		static std::pair<bool, std::vector<source>> decode_singlethread(compiler_metadata& metadata);
		static bool decode_source(source& source, compiler_metadata& metadata);

	private:
		static bool decode_utf16_(const endian encoding_endian, const std::fpos_t length, const encoding detected_encoding,
								  std::ifstream& stream, source& source, compiler_metadata& metadata);
		static bool decode_utf32_(const endian encoding_endian, const std::fpos_t length, const encoding detected_encoding,
								  std::ifstream& stream, source& source, compiler_metadata& metadata);
	};
}

#endif