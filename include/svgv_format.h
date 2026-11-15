#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace vv {
inline constexpr std::array<std::uint8_t, 8> kSvgcMagic{'S','V','G','C','\r','\n',0x1a,'\n'};
inline constexpr std::array<std::uint8_t, 8> kSvgvMagic{'S','V','G','V','\r','\n',0x1a,'\n'};
inline constexpr std::uint16_t kVersionMajor = 1, kVersionMinor = 0, kBezierProfile = 1;
inline constexpr std::uint32_t kMaxDimension = 16384;

#pragma pack(push, 1)
struct FileHeaderV1 {
    std::uint8_t magic[8];
    std::uint16_t major, minor, header_bytes;
    std::uint8_t quality, flags;
    std::uint16_t profile, reserved;
    std::uint32_t width, height, timebase_num, timebase_den;
    std::uint64_t payload_bytes, raw_payload_bytes;
    std::uint32_t payload_crc32, header_crc32;
};
#pragma pack(pop)
static_assert(sizeof(FileHeaderV1) == 60);
static_assert(offsetof(FileHeaderV1, width) == 20);
static_assert(offsetof(FileHeaderV1, payload_crc32) == 52);
static_assert(offsetof(FileHeaderV1, header_crc32) == 56);

struct Point { std::int32_t x = 0, y = 0; bool operator==(const Point&) const = default; }; // signed 16.16 pixels
struct Cubic { Point c1, c2, end; bool operator==(const Cubic&) const = default; };
struct Path { std::array<std::uint8_t, 4> rgba{0,0,0,255}; bool closed = true; Point start; std::vector<Cubic> curves; bool operator==(const Path&) const = default; };
struct Scene { std::uint32_t width = 0, height = 0; std::uint8_t quality = 100; std::vector<Path> paths; bool operator==(const Scene&) const = default; };
struct Frame { std::uint64_t timestamp = 0; Scene scene; bool operator==(const Frame&) const = default; };
struct Video { std::uint32_t width = 0, height = 0, timebase_num = 1, timebase_den = 1000000; std::uint8_t quality = 100; std::vector<Frame> frames; bool operator==(const Video&) const = default; };

std::uint32_t crc32_ieee(std::span<const std::uint8_t> bytes) noexcept;
std::vector<std::uint8_t> encode_svgc(const Scene& scene);
Scene decode_svgc(std::span<const std::uint8_t> file);
std::vector<std::uint8_t> encode_svgv(const Video& video);
Video decode_svgv(std::span<const std::uint8_t> file);
} // namespace vv
