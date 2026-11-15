#include "image_io.h"
#include <cstdio>
#include <png.h>
#include <jpeglib.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <zlib.h>

namespace bench { namespace {
void same(const Image&a,const Image&b){if(a.width!=b.width||a.height!=b.height||a.rgba.size()!=b.rgba.size())throw std::runtime_error("image size mismatch");}
double lum(const std::uint8_t*p){return .2126*p[0]+.7152*p[1]+.0722*p[2];}
bool inside(double x,double y,const std::vector<std::pair<double,double>>&p){bool in=false;for(std::size_t i=0,j=p.size()-1;i<p.size();j=i++)if(((p[i].second>y)!=(p[j].second>y))&&(x<(p[j].first-p[i].first)*(y-p[i].second)/(p[j].second-p[i].second)+p[i].first))in=!in;return in;}
}
Image rasterize(const vv::Scene&s){Image im{s.width,s.height,std::vector<std::uint8_t>(std::size_t(s.width)*s.height*4,255)};
 for(const auto&path:s.paths){if(!path.closed||path.curves.empty())continue;std::vector<std::pair<double,double>> p;auto cv=[](vv::Point a){return std::pair<double,double>{a.x/65536.0,a.y/65536.0};};
  auto prev=path.start;p.push_back(cv(prev));for(const auto&c:path.curves){auto a=cv(prev),b=cv(c.c1),d=cv(c.c2),e=cv(c.end);for(int i=1;i<=12;++i){double t=i/12.0,u=1-t;p.emplace_back(u*u*u*a.first+3*u*u*t*b.first+3*u*t*t*d.first+t*t*t*e.first,u*u*u*a.second+3*u*u*t*b.second+3*u*t*t*d.second+t*t*t*e.second);}prev=c.end;}
  for(std::uint32_t y=0;y<s.height;++y)for(std::uint32_t x=0;x<s.width;++x)if(inside(x+.5,y+.5,p)){auto k=(std::size_t(y)*s.width+x)*4;unsigned a=path.rgba[3];for(int c=0;c<3;++c)im.rgba[k+c]=std::uint8_t((path.rgba[c]*a+im.rgba[k+c]*(255-a)+127)/255);im.rgba[k+3]=255;}
 }return im;}
double psnr(const Image&a,const Image&b){same(a,b);long double e=0;std::size_t n=0;for(std::size_t i=0;i<a.rgba.size();i+=4)for(int c=0;c<3;++c){double d=int(a.rgba[i+c])-int(b.rgba[i+c]);e+=d*d;++n;}double mse=double(e/n);return mse==0?INFINITY:10*std::log10(255.0*255.0/mse);}
double ssim8x8(const Image&a,const Image&b){same(a,b);constexpr double c1=6.5025,c2=58.5225;double sum=0;std::size_t blocks=0;
 for(std::uint32_t y0=0;y0<a.height;y0+=8)for(std::uint32_t x0=0;x0<a.width;x0+=8){std::vector<double>x,y;for(std::uint32_t j=y0;j<std::min(y0+8,a.height);++j)for(std::uint32_t i=x0;i<std::min(x0+8,a.width);++i){auto k=(std::size_t(j)*a.width+i)*4;x.push_back(lum(&a.rgba[k]));y.push_back(lum(&b.rgba[k]));}double mx=0,my=0;for(std::size_t k=0;k<x.size();++k){mx+=x[k];my+=y[k];}mx/=x.size();my/=y.size();double vx=0,vy=0,cov=0;for(std::size_t k=0;k<x.size();++k){double dx=x[k]-mx,dy=y[k]-my;vx+=dx*dx;vy+=dy*dy;cov+=dx*dy;}vx/=x.size();vy/=y.size();cov/=x.size();sum+=((2*mx*my+c1)*(2*cov+c2))/((mx*mx+my*my+c1)*(vx+vy+c2));++blocks;}return sum/blocks;}
std::vector<std::uint8_t> png_encode(const Image&i){png_image p{};p.version=PNG_IMAGE_VERSION;p.width=i.width;p.height=i.height;p.format=PNG_FORMAT_RGBA;png_alloc_size_t n=0;if(!png_image_write_to_memory(&p,nullptr,&n,0,i.rgba.data(),0,nullptr))throw std::runtime_error("PNG encode failed");std::vector<std::uint8_t>b(n);if(!png_image_write_to_memory(&p,b.data(),&n,0,i.rgba.data(),0,nullptr))throw std::runtime_error("PNG encode failed");b.resize(n);return b;}
Image png_decode(const std::vector<std::uint8_t>&b){png_image p{};p.version=PNG_IMAGE_VERSION;if(!png_image_begin_read_from_memory(&p,b.data(),b.size()))throw std::runtime_error("PNG decode failed");p.format=PNG_FORMAT_RGBA;Image i{p.width,p.height,std::vector<std::uint8_t>(PNG_IMAGE_SIZE(p))};if(!png_image_finish_read(&p,nullptr,i.rgba.data(),0,nullptr)){png_image_free(&p);throw std::runtime_error("PNG decode failed");}png_image_free(&p);return i;}
std::vector<std::uint8_t> jpeg_encode(const Image&i,int quality){jpeg_compress_struct c{};jpeg_error_mgr e{};c.err=jpeg_std_error(&e);jpeg_create_compress(&c);unsigned char*out=nullptr;unsigned long n=0;jpeg_mem_dest(&c,&out,&n);c.image_width=i.width;c.image_height=i.height;c.input_components=3;c.in_color_space=JCS_RGB;jpeg_set_defaults(&c);jpeg_set_quality(&c,quality,TRUE);jpeg_start_compress(&c,TRUE);std::vector<std::uint8_t>row(std::size_t(i.width)*3);while(c.next_scanline<c.image_height){auto y=c.next_scanline;for(std::uint32_t x=0;x<i.width;++x)for(int k=0;k<3;++k)row[x*3+k]=i.rgba[(std::size_t(y)*i.width+x)*4+k];JSAMPROW r=row.data();jpeg_write_scanlines(&c,&r,1);}jpeg_finish_compress(&c);std::vector<std::uint8_t>b(out,out+n);free(out);jpeg_destroy_compress(&c);return b;}
Image jpeg_decode(const std::vector<std::uint8_t>&b){jpeg_decompress_struct c{};jpeg_error_mgr e{};c.err=jpeg_std_error(&e);jpeg_create_decompress(&c);jpeg_mem_src(&c,b.data(),b.size());jpeg_read_header(&c,TRUE);c.out_color_space=JCS_RGB;jpeg_start_decompress(&c);Image i{c.output_width,c.output_height,std::vector<std::uint8_t>(std::size_t(c.output_width)*c.output_height*4,255)};std::vector<std::uint8_t>row(std::size_t(c.output_width)*3);while(c.output_scanline<c.output_height){auto y=c.output_scanline;JSAMPROW r=row.data();jpeg_read_scanlines(&c,&r,1);for(std::uint32_t x=0;x<i.width;++x)for(int k=0;k<3;++k)i.rgba[(std::size_t(y)*i.width+x)*4+k]=row[x*3+k];}jpeg_finish_decompress(&c);jpeg_destroy_decompress(&c);return i;}
std::vector<std::uint8_t> residual_encode(const Image& ref,const Image& test){
 same(ref,test);std::vector<std::uint8_t>raw(ref.rgba.size()*2);
 for(std::size_t i=0;i<ref.rgba.size();++i){auto d=int(ref.rgba[i])-int(test.rgba[i]);auto u=static_cast<std::uint16_t>(d);raw[2*i]=u;raw[2*i+1]=u>>8;}
 uLongf n=compressBound(raw.size());std::vector<std::uint8_t>z(n);
 if(compress2(z.data(),&n,raw.data(),raw.size(),Z_BEST_COMPRESSION)!=Z_OK)throw std::runtime_error("residual compression failed");z.resize(n);return z;
}
Image residual_decode(const Image& base,const std::vector<std::uint8_t>& residual){
 std::vector<std::uint8_t>raw(base.rgba.size()*2);uLongf n=raw.size();
 if(uncompress(raw.data(),&n,residual.data(),residual.size())!=Z_OK||n!=raw.size())throw std::runtime_error("residual decode failed");
 Image out=base;for(std::size_t i=0;i<base.rgba.size();++i){unsigned u=raw[2*i]|(unsigned(raw[2*i+1])<<8);int d=u<32768?int(u):int(u)-65536;int v=int(base.rgba[i])+d;if(v<0||v>255)throw std::runtime_error("residual sample out of range");out.rgba[i]=static_cast<std::uint8_t>(v);}return out;
}
} // namespace bench
