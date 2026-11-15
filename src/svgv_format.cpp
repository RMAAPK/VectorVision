#include "svgv_format.h"
#include <algorithm>
#include <bit>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <zlib.h>

namespace vv { namespace {
using Bytes = std::vector<std::uint8_t>;
constexpr std::uint64_t kMaxPayload = 256ull * 1024 * 1024;

void need(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
void put8(Bytes& b, std::uint8_t v) { b.push_back(v); }
void put16(Bytes& b, std::uint16_t v) { for (int i=0;i<2;++i) b.push_back(v >> (8*i)); }
void put32(Bytes& b, std::uint32_t v) { for (int i=0;i<4;++i) b.push_back(v >> (8*i)); }
void put64(Bytes& b, std::uint64_t v) { for (int i=0;i<8;++i) b.push_back(v >> (8*i)); }

struct Reader {
    std::span<const std::uint8_t> b; std::size_t p = 0;
    void need_n(std::size_t n) { need(n <= b.size()-p, "truncated payload"); }
    std::uint8_t u8() { need_n(1); return b[p++]; }
    std::uint16_t u16() { std::uint16_t v=0; for(int i=0;i<2;++i) v |= std::uint16_t(u8()) << (8*i); return v; }
    std::uint32_t u32() { std::uint32_t v=0; for(int i=0;i<4;++i) v |= std::uint32_t(u8()) << (8*i); return v; }
    std::uint64_t u64() { std::uint64_t v=0; for(int i=0;i<8;++i) v |= std::uint64_t(u8()) << (8*i); return v; }
    std::int32_t i32() { return std::bit_cast<std::int32_t>(u32()); }
    std::span<const std::uint8_t> take(std::size_t n) { need_n(n); auto s=b.subspan(p,n); p+=n; return s; }
};

Bytes scene_payload(const Scene& s) {
    need(s.paths.size() <= 1000000, "too many paths");
    Bytes b; put32(b, static_cast<std::uint32_t>(s.paths.size()));
    for (const auto& p : s.paths) {
        need(p.curves.size() <= 1000000, "too many curves");
        b.insert(b.end(), p.rgba.begin(), p.rgba.end()); put8(b, p.closed ? 1 : 0);
        put32(b, static_cast<std::uint32_t>(p.curves.size()));
        put32(b, static_cast<std::uint32_t>(p.start.x)); put32(b, static_cast<std::uint32_t>(p.start.y));
        for (const auto& c : p.curves) for (auto v : {c.c1.x,c.c1.y,c.c2.x,c.c2.y,c.end.x,c.end.y}) put32(b, static_cast<std::uint32_t>(v));
    }
    need(b.size() <= kMaxPayload, "scene payload too large"); return b;
}
Scene read_scene(std::span<const std::uint8_t> b, std::uint32_t w, std::uint32_t h, std::uint8_t q) {
    Reader r{b}; Scene s{w,h,q,{}}; auto n=r.u32(); need(n<=1000000, "too many paths"); s.paths.reserve(n);
    for (std::uint32_t i=0;i<n;++i) {
        Path p; auto col=r.take(4); std::copy(col.begin(),col.end(),p.rgba.begin());
        auto closed=r.u8(); need(closed<=1, "invalid path flags"); p.closed=closed!=0;
        auto m=r.u32(); need(m<=1000000, "too many curves"); p.start={r.i32(),r.i32()}; p.curves.reserve(m);
        for(std::uint32_t j=0;j<m;++j) p.curves.push_back({{r.i32(),r.i32()},{r.i32(),r.i32()},{r.i32(),r.i32()}});
        s.paths.push_back(std::move(p));
    }
    need(r.p==b.size(), "trailing scene data"); return s;
}
Bytes compress_bytes(std::span<const std::uint8_t> raw) {
    uLongf n=compressBound(static_cast<uLong>(raw.size())); Bytes out(n);
    need(compress2(out.data(),&n,raw.data(),static_cast<uLong>(raw.size()),Z_BEST_COMPRESSION)==Z_OK,"zlib compression failed");
    out.resize(n); return out;
}
Bytes decompress_bytes(std::span<const std::uint8_t> data, std::uint64_t raw_size) {
    need(raw_size<=kMaxPayload,"payload exceeds limit"); Bytes out(static_cast<std::size_t>(raw_size)); uLongf n=out.size();
    need(uncompress(out.data(),&n,data.data(),static_cast<uLong>(data.size()))==Z_OK && n==raw_size,"invalid compressed payload"); return out;
}
std::uint16_t le16(std::uint16_t v) { if constexpr(std::endian::native==std::endian::little) return v; return (v>>8)|(v<<8); }
std::uint32_t le32(std::uint32_t v) { if constexpr(std::endian::native==std::endian::little) return v; return (v>>24)|((v>>8)&0x0000ff00u)|((v<<8)&0x00ff0000u)|(v<<24); }
std::uint64_t le64(std::uint64_t v) { if constexpr(std::endian::native==std::endian::little) return v; return (std::uint64_t(le32(v))<<32)|le32(v>>32); }

Bytes wrap_file(const std::array<std::uint8_t,8>& magic, std::uint32_t w, std::uint32_t h, std::uint8_t q,
                std::uint32_t tn, std::uint32_t td, std::span<const std::uint8_t> raw) {
    need(w>0&&h>0&&w<=kMaxDimension&&h<=kMaxDimension,"invalid dimensions"); need(q<=100,"quality out of range");
    auto compressed=compress_bytes(raw); need(compressed.size()<=kMaxPayload,"compressed payload too large"); FileHeaderV1 x{}; std::copy(magic.begin(),magic.end(),x.magic);
    x.major=le16(kVersionMajor); x.minor=le16(kVersionMinor); x.header_bytes=le16(sizeof(x)); x.quality=q;
    x.profile=le16(kBezierProfile); x.width=le32(w); x.height=le32(h); x.timebase_num=le32(tn); x.timebase_den=le32(td);
    x.payload_bytes=le64(compressed.size()); x.raw_payload_bytes=le64(raw.size()); x.payload_crc32=le32(crc32_ieee(compressed));
    x.header_crc32=le32(crc32_ieee({reinterpret_cast<const std::uint8_t*>(&x),offsetof(FileHeaderV1,header_crc32)}));
    Bytes out(sizeof(x)+compressed.size()); std::memcpy(out.data(),&x,sizeof(x)); std::copy(compressed.begin(),compressed.end(),out.begin()+sizeof(x)); return out;
}
struct Opened { FileHeaderV1 h; Bytes raw; };
Opened open_file(std::span<const std::uint8_t> file, const std::array<std::uint8_t,8>& magic) {
    need(file.size()>=sizeof(FileHeaderV1),"truncated header"); FileHeaderV1 w{}; std::memcpy(&w,file.data(),sizeof(w));
    need(std::equal(magic.begin(),magic.end(),w.magic),"wrong magic");
    need(le32(w.header_crc32)==crc32_ieee(file.first(offsetof(FileHeaderV1,header_crc32))),"header CRC mismatch");
    FileHeaderV1 h=w; h.major=le16(w.major); h.minor=le16(w.minor); h.header_bytes=le16(w.header_bytes); h.profile=le16(w.profile); h.reserved=le16(w.reserved);
    h.width=le32(w.width); h.height=le32(w.height); h.timebase_num=le32(w.timebase_num); h.timebase_den=le32(w.timebase_den);
    h.payload_bytes=le64(w.payload_bytes); h.raw_payload_bytes=le64(w.raw_payload_bytes); h.payload_crc32=le32(w.payload_crc32);
    need(h.major==kVersionMajor&&h.minor==kVersionMinor&&h.header_bytes==sizeof(h),"unsupported version/header");
    need(h.quality<=100&&w.flags==0&&h.profile==kBezierProfile&&h.reserved==0,"unsupported header fields");
    need(h.width&&h.height&&h.width<=kMaxDimension&&h.height<=kMaxDimension,"invalid dimensions");
    need(h.payload_bytes<=kMaxPayload&&h.raw_payload_bytes<=kMaxPayload,"payload exceeds limit");
    need(h.payload_bytes==file.size()-sizeof(h),"payload length mismatch"); auto enc=file.subspan(sizeof(h));
    need(crc32_ieee(enc)==h.payload_crc32,"payload CRC mismatch"); return {h,decompress_bytes(enc,h.raw_payload_bytes)};
}
} // namespace

std::uint32_t crc32_ieee(std::span<const std::uint8_t> b) noexcept {
    std::uint32_t c=0xffffffffu; for(auto x:b){c^=x;for(int i=0;i<8;++i)c=(c>>1)^(0xedb88320u&-(c&1u));} return c^0xffffffffu;
}
std::vector<std::uint8_t> encode_svgc(const Scene& s) {
    need(s.quality<=100,"quality out of range"); return wrap_file(kSvgcMagic,s.width,s.height,s.quality,0,0,scene_payload(s));
}
Scene decode_svgc(std::span<const std::uint8_t> f) {
    auto o=open_file(f,kSvgcMagic); need(o.h.timebase_num==0&&o.h.timebase_den==0,"still image has timebase");
    return read_scene(o.raw,o.h.width,o.h.height,o.h.quality);
}
std::vector<std::uint8_t> encode_svgv(const Video& v) {
    need(v.quality<=100&&v.frames.size()<=1000000&&v.timebase_num&&v.timebase_den,"invalid video metadata"); Bytes raw; put32(raw,static_cast<std::uint32_t>(v.frames.size()));
    std::uint64_t last=0; bool first=true;
    for(const auto& f:v.frames){need(first||f.timestamp>last,"timestamps must increase"); first=false; last=f.timestamp;
        need(f.scene.width==v.width&&f.scene.height==v.height&&f.scene.quality==v.quality,"frame metadata mismatch");
        auto b=scene_payload(f.scene); put64(raw,f.timestamp); put64(raw,b.size()); raw.insert(raw.end(),b.begin(),b.end());}
    return wrap_file(kSvgvMagic,v.width,v.height,v.quality,v.timebase_num,v.timebase_den,raw);
}
Video decode_svgv(std::span<const std::uint8_t> f) {
    auto o=open_file(f,kSvgvMagic); need(o.h.timebase_num&&o.h.timebase_den,"video timebase missing"); Reader r{o.raw};
    auto n=r.u32(); need(n<=1000000,"too many frames"); Video v{o.h.width,o.h.height,o.h.timebase_num,o.h.timebase_den,o.h.quality,{}}; v.frames.reserve(n);
    std::uint64_t last=0; bool first=true;
    for(std::uint32_t i=0;i<n;++i){auto t=r.u64(),size=r.u64(); need(first||t>last,"timestamps must increase"); first=false; last=t;
        need(size<=kMaxPayload,"frame payload too large"); v.frames.push_back({t,read_scene(r.take(static_cast<std::size_t>(size)),v.width,v.height,v.quality)});}
    need(r.p==r.b.size(),"trailing video data"); return v;
}
} // namespace vv
