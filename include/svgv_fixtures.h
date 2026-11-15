#pragma once
#include "svgv_format.h"
#include <cmath>
#include <stdexcept>

namespace vv::fixtures {
inline std::int32_t fixed(double x) { return static_cast<std::int32_t>(std::llround(x*65536.0)); }
inline void line_to(Path& p, Point a, Point b) {
    auto dx=std::int64_t(b.x)-a.x, dy=std::int64_t(b.y)-a.y;
    p.curves.push_back({{static_cast<std::int32_t>(a.x+dx/3),static_cast<std::int32_t>(a.y+dy/3)},
                        {static_cast<std::int32_t>(a.x+2*dx/3),static_cast<std::int32_t>(a.y+2*dy/3)},b});
}
inline Path polygon(const std::vector<Point>& pts, std::array<std::uint8_t,4> rgba) {
    if(pts.size()<3) throw std::invalid_argument("polygon needs 3 points");
    Path p; p.rgba=rgba; p.start=pts.front();
    for(std::size_t i=1;i<=pts.size();++i) line_to(p,pts[(i-1)%pts.size()],pts[i%pts.size()]);
    return p;
}
inline Scene make_fixture(std::uint32_t w=128,std::uint32_t h=128,std::uint8_t q=100,double shift=0) {
    if(q>100||w<64||h<64) throw std::invalid_argument("fixture bounds");
    Scene s{w,h,q,{}};
    s.paths.push_back(polygon({{fixed(8),fixed(8)},{fixed(42),fixed(8)},{fixed(42),fixed(38)},{fixed(8),fixed(38)}},{220,45,40,255}));
    const unsigned spikes=4+(28u*q+50u)/100u; const double cx=w*0.66+shift,cy=h*0.60;
    std::vector<Point> star; star.reserve(spikes*2);
    for(unsigned i=0;i<spikes*2;++i){double a=-1.5707963267948966+i*3.1415926535897932/spikes;
        double r=(i&1)?h*0.13:h*0.31; star.push_back({fixed(cx+r*std::cos(a)),fixed(cy+r*std::sin(a))});}
    s.paths.push_back(polygon(star,{30,95,210,255})); return s;
}
inline Video make_video(std::uint8_t q=100) {
    Video v{128,128,1,1000000,q,{}};
    for(std::uint64_t i=0;i<5;++i) v.frames.push_back({i*40000,make_fixture(128,128,q,double(i)*2.0)});
    return v;
}
} // namespace vv::fixtures
