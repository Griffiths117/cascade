// cascade — I/O dispatch and format detection
// SPDX-License-Identifier: Apache-2.0
#include "cascade/io/format.hpp"
#include "cascade/io/sdatb.hpp"
#include "cascade/io/touchstone.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace cascade::io {

namespace {
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::vector<unsigned char> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f)
        throw UnknownFormat("cannot open file: " + path);
    std::vector<unsigned char> buf((std::istreambuf_iterator<char>(f)),
                                   std::istreambuf_iterator<char>());
    return buf;
}

std::uint32_t read_u32_le(const unsigned char* p) {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}

// Read a 7-bit encoded integer (as .NET BinaryReader).
std::size_t read_vint(const unsigned char* data, std::size_t size, std::size_t& off) {
    std::size_t shift = 0;
    std::size_t value = 0;
    while (off < size) {
        unsigned char b = data[off++];
        value |= static_cast<std::size_t>(b & 0x7F) << shift;
        if ((b & 0x80) == 0)
            return value;
        shift += 7;
        if (shift >= 64)
            throw MalformedFile("invalid 7-bit integer");
    }
    throw MalformedFile("truncated 7-bit integer");
}

std::string read_dotnet_string(const unsigned char* data, std::size_t size, std::size_t& off) {
    const std::size_t len = read_vint(data, size, off);
    if (off + len > size)
        throw MalformedFile("truncated string");
    std::string s(reinterpret_cast<const char*>(data + off), len);
    off += len;
    return s;
}
}  // namespace

int sdatb_format_version(const std::string& path) {
    const auto buf = read_file(path);
    if (buf.size() < 4)
        throw MalformedFile("file too short: " + path);
    // v1 is gzip-compressed.
    if (buf[0] == 0x1f && buf[1] == 0x8b)
        return 1;

    std::size_t off = 0;
    const std::string header = read_dotnet_string(buf.data(), buf.size(), off);
    if (header != "%SDATA")
        throw MalformedFile("not an sdatb file: " + path);
    if (off + 4 > buf.size())
        throw MalformedFile("truncated version: " + path);
    const int version = static_cast<int>(read_u32_le(buf.data() + off));
    if (version < 2 || version > 5)
        throw MalformedFile("unsupported sdatb version");
    return version;
}

FileFormat detect_format(const std::string& path) {
    namespace fs = std::filesystem;
    const std::string ext = lower(fs::path(path).extension().string());

    FileFormat by_ext = FileFormat::auto_detect;
    if (ext == ".sdatb")
        by_ext = FileFormat::sdatb;
    else if (ext == ".s1p" || ext == ".s2p" || ext == ".snp")
        by_ext = FileFormat::touchstone_v1;
    else if (ext == ".ts")
        by_ext = FileFormat::touchstone_v2;
    else if (ext == ".citi")
        by_ext = FileFormat::citi;

    const auto buf = read_file(path);
    if (buf.size() >= 2 && buf[0] == 0x1f && buf[1] == 0x8b) {
        // gzip: treat as sdatb v1 (version validated by the reader).
        return FileFormat::sdatb;
    }
    // Raw sdatb: validate the header/version now rather than guessing.
    if (buf.size() >= 8) {
        std::size_t off = 0;
        try {
            const std::string header = read_dotnet_string(buf.data(), buf.size(), off);
            if (header == "%SDATA") {
                if (off + 4 <= buf.size()) {
                    const int v = static_cast<int>(read_u32_le(buf.data() + off));
                    if (v >= 1 && v <= 5)
                        return FileFormat::sdatb;
                }
                throw MalformedFile("invalid sdatb version in: " + path);
            }
        } catch (const MalformedFile&) {
            // Fall through to other format checks.
        }
    }
    // Content sniffing for other formats.
    std::string head(reinterpret_cast<const char*>(buf.data()),
                     std::min<std::size_t>(buf.size(), 256));
    if (head.find("CITIFILE") != std::string::npos)
        return FileFormat::citi;
    if (head.find("#") != std::string::npos) {
        return (by_ext == FileFormat::touchstone_v2) ? FileFormat::touchstone_v2
                                                     : FileFormat::touchstone_v1;
    }

    if (by_ext != FileFormat::auto_detect)
        return by_ext;
    throw UnknownFormat("unrecognised file format: " + path);
}

SData read_dataset(const std::string& path, FileFormat format) {
    if (format == FileFormat::auto_detect)
        format = detect_format(path);
    switch (format) {
    case FileFormat::sdatb:
        return read_sdatb(path);
    case FileFormat::touchstone_v1:
    case FileFormat::touchstone_v2:
        return read_touchstone(path);
    case FileFormat::citi:
        throw UnsupportedFeature("CITI reader not implemented yet");
    case FileFormat::auto_detect:
        throw UnknownFormat("auto_detect did not resolve: " + path);
    }
    throw UnknownFormat("unhandled format for: " + path);
}

SData read_touchstone(const std::string& /*path*/) {
    throw UnsupportedFeature("Touchstone reader not implemented yet");
}

}  // namespace cascade::io
