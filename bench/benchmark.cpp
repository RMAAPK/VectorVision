#include "image_io.h"
#include "svgv_fixtures.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using Clock=std::chrono::steady_clock;
template<class F> double timed_ms(F fn,int repetitions=8){auto a=Clock::now();for(int i=0;i<repetitions;++i)fn();auto b=Clock::now();return std::chrono::duration<double,std::milli>(b-a).count()/repetitions;}
int main(int argc,char**argv){
 try{
  bool residual=false;std::string csv="build/benchmark.csv";
  for(int i=1;i<argc;++i){std::string a=argv[i];if(a=="--experimental-residual")residual=true;else if(a=="--csv"&&i+1<argc)csv=argv[++i];else throw std::runtime_error("usage: svgv_bench [--experimental-residual] [--csv FILE]");}
  std::ofstream out(csv);if(!out)throw std::runtime_error("cannot open CSV output");
  out<<"codec,quality,bytes,psnr_db,ssim8x8,encode_ms,decode_ms,render_ms,experimental_residual_bytes\n";
  const auto source=bench::rasterize(vv::fixtures::make_fixture(128,128,100));
  const auto png=bench::png_encode(source);bench::Image png_decoded;
  double png_enc=timed_ms([&]{auto b=bench::png_encode(source);(void)b;});
  double png_dec=timed_ms([&]{png_decoded=bench::png_decode(png);});
  out<<"PNG,100,"<<png.size()<<','<<bench::psnr(source,png_decoded)<<','<<bench::ssim8x8(source,png_decoded)<<','<<png_enc<<','<<png_dec<<",0,0\n";
  for(int q:{0,25,50,75,100}){
   auto candidate=vv::fixtures::make_fixture(128,128,static_cast<std::uint8_t>(q));std::vector<std::uint8_t> file;
   double enc=timed_ms([&]{file=vv::encode_svgc(candidate);});vv::Scene decoded;
   double dec=timed_ms([&]{decoded=vv::decode_svgc(file);});bench::Image image;double render=timed_ms([&]{image=bench::rasterize(decoded);});
   out<<"SVGC,"<<q<<','<<file.size()<<','<<bench::psnr(source,image)<<','<<bench::ssim8x8(source,image)<<','<<enc<<','<<dec<<','<<render<<",0\n";
   if(residual){
    std::vector<std::uint8_t> residual_bytes;double residual_enc=timed_ms([&]{residual_bytes=bench::residual_encode(source,image);});bench::Image hybrid;
    double residual_dec=timed_ms([&]{hybrid=bench::residual_decode(image,residual_bytes);});if(hybrid.rgba!=source.rgba)throw std::runtime_error("experimental residual parity failed");
    out<<"HYBRID_EXPERIMENTAL,"<<q<<','<<file.size()+residual_bytes.size()+16<<','<<bench::psnr(source,hybrid)<<','<<bench::ssim8x8(source,hybrid)<<','<<enc+residual_enc<<','<<dec+residual_dec<<','<<render<<','<<residual_bytes.size()<<'\n';
   }
  }
  for(int q:{30,50,70,90,95}){
   std::vector<std::uint8_t> file;double enc=timed_ms([&]{file=bench::jpeg_encode(source,q);});bench::Image image;
   double dec=timed_ms([&]{image=bench::jpeg_decode(file);});
   out<<"JPEG,"<<q<<','<<file.size()<<','<<bench::psnr(source,image)<<','<<bench::ssim8x8(source,image)<<','<<enc<<','<<dec<<",0,0\n";
  }
  if(residual)std::cerr<<"experimental residual sizes are estimates only; residual data is not written to SVGC/SVGV files\n";
  std::cout<<"wrote "<<csv<<" (PNG/JPEG raster baselines, SVGC rate-distortion points)\n";return 0;
 }catch(const std::exception&e){std::cerr<<"benchmark error: "<<e.what()<<'\n';return 1;}
}
