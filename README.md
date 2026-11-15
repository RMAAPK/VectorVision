# VectorVision

Vector-only `.svgc` still scenes and `.svgv` timestamped vector-frame sequences.

This v1 foundation is a reference binary format/codec, not yet a raster-to-curve encoder. `.svgv` currently stores full scenes per frame; temporal delta coding is future work. The residual option is benchmark-only and never enters the core bitstream.

## Build and test

Requires C++20, CMake, zlib, libpng, and libjpeg.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/svgv_bench --csv build/benchmark.csv
./build/svgv_bench --experimental-residual --csv build/benchmark-residual.csv
```

Synthetic benchmark results are not evidence of a general compression win; test real, licensed corpora before making performance claims.
