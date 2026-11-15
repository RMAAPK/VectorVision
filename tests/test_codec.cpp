#include "svgv_fixtures.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <stdexcept>

using namespace vv;
static void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
template<class F> static void rejects(F f){bool threw=false;try{f();}catch(const std::exception&){threw=true;}check(threw,"malformed file accepted");}
int main(){
    try {
        const std::string known="123456789";
        check(crc32_ieee({reinterpret_cast<const std::uint8_t*>(known.data()),known.size()})==0xcbf43926u,"CRC-32/IEEE vector");
        auto s=fixtures::make_fixture(128,128,73); auto enc=encode_svgc(s);
        check(std::equal(kSvgcMagic.begin(),kSvgcMagic.end(),enc.begin()),"SVGC signature");
        check(decode_svgc(enc)==s,"SVGC bit-exact round trip");
        auto v=fixtures::make_video(61); auto vbytes=encode_svgv(v);
        check(std::equal(kSvgvMagic.begin(),kSvgvMagic.end(),vbytes.begin()),"SVGV signature");
        check(decode_svgv(vbytes)==v,"SVGV bit-exact round trip");
        rejects([&]{decode_svgc(std::span<const std::uint8_t>(enc).first(12));});
        auto bad=enc; bad[0]^=1; rejects([&]{decode_svgc(bad);});
        bad=enc; bad.back()^=1; rejects([&]{decode_svgc(bad);});
        auto empty_video=Video{128,128,1,1000000,100,{}};
        check(decode_svgv(encode_svgv(empty_video))==empty_video,"empty SVGV round trip");
        std::cout<<"all codec checks passed\n"; return 0;
    } catch(const std::exception& e){std::cerr<<"test failure: "<<e.what()<<'\n';return 1;}
}
