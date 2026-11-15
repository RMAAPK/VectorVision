#pragma once
#include "svgv_format.h"
#include <cstdint>
#include <vector>

namespace bench {
struct Image { std::uint32_t width=0,height=0; std::vector<std::uint8_t> rgba; };
Image rasterize(const vv::Scene& scene);
double psnr(const Image& ref,const Image& test);
double ssim8x8(const Image& ref,const Image& test);
std::vector<std::uint8_t> png_encode(const Image& image);
Image png_decode(const std::vector<std::uint8_t>& bytes);
std::vector<std::uint8_t> jpeg_encode(const Image& image,int quality);
Image jpeg_decode(const std::vector<std::uint8_t>& bytes);
std::vector<std::uint8_t> residual_encode(const Image& ref,const Image& test);
Image residual_decode(const Image& base,const std::vector<std::uint8_t>& residual);
} // namespace bench
