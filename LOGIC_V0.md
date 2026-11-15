# VectorVision codec logic — v0 proposal

**Status:** local C++20 reference format/codec and synthetic benchmark are scaffolded; compilation/CI are not yet verified. Raster-to-curve encoding and calibrated quality mapping are not implemented.

## Core claim to test

Represent image structure as colored regions bounded by curves, with a user-controlled fidelity/size tradeoff. Curves can render at different output sizes, but they do not recover source detail that the encoder discarded. The success criterion is smaller files at comparable reconstructed quality on a declared workload—not “vectors are infinite-resolution, therefore smaller.”

## Still-image path (`.svgc`) first

1. Decode source and preserve its reference width, height, color space, and alpha semantics.
2. Extract candidate regions/contours. Fit cubic Bézier segments with a controllable maximum geometric deviation; retain holes and boundaries explicitly.
3. Quantize coordinates and region colors under explicit bounds. Encode curve commands, fills, and metadata in a compact binary payload; choose the entropy/compression layer only after testing candidates.
4. Decode and rasterize at the reference dimensions. Measure distortion against the source.
5. Select the smallest candidate that meets the requested fidelity target. If it cannot meet the target, report that; do not silently call it “high quality.”

A user-facing quality setting can be a calibrated 0–100 control: increasing it should generally reduce curve error, preserve smaller features, and use more color precision/regions. That mapping must be calibrated by corpus and versioned. The file should record the encoder/profile and effective parameters (such as curve tolerance and color quantization), not rely on a vague slider number alone. `100` means only the highest defined fidelity mode; it is lossless only if the encoding actually preserves the source exactly.

## Video path (`.svgv`) after the still coder

A video is a timed sequence of vector scenes, not a bag of unrelated pictures. Reuse the still-scene model for keyframes, then track stable region/path IDs between frames and encode curve/control-point deltas where correspondence is reliable. Encode explicit create/delete/split/merge events and new keyframes when topology, occlusion, or tracking changes make deltas unsafe. Store timestamps/timebase. Evaluate both per-frame fidelity and temporal instability/flicker. Do not make video depend on an unproven still-image representation without a fallback or a measured quality result.

## Quality/rate objective

The engineering objective is:

`minimize encoded bytes subject to a declared distortion target at the source dimensions`.

The v1 reference encoder currently serializes supplied cubic paths; it does not extract paths from raster images. Its quality byte records the request but the general mapping is not yet normative.

For stills, compare at least pixel/perceptual and edge/shape fidelity; for video, also measure temporal consistency. Name and version the metrics and use visual review, since no single metric captures every artifact. Keep the user setting separate from the measured distortion and actual encoded parameters.

## Benchmark gate before claims

- Test separate classes: flat illustrations/line art, screenshots/UI, natural photographs, and representative video scenes.
- Compare stills with strong contemporary still-image codecs and video with strong contemporary video codecs, at matched quality and output dimensions. Also record bytes, encode/decode time, memory, and rendering cost.
- Include failures: thin lines, text, gradients, foliage/hair, noise, transparency, fast motion, occlusion, and scene cuts.
- Publish corpus provenance, encoder settings, output files, and measured results. Do not repeat the archived “85%” claim unless the benchmark supports it.

## Scope discipline

This proposal prioritizes `.svgc` because `.svgv` depends on a usable frame representation. It does not assume the product is limited to graphics: all content classes above are test candidates. The next design review must choose the initial workload, metric/profile, and whether texture can be represented only by curves or needs an explicitly declared hybrid mechanism. No implementation should imply that these open choices are already solved.

## Build/CI status

The CircleCI account connection works, but `gh/RMAAPK/VectorVision` currently has zero runs and the repository is empty upstream. Add a CI config once a reference implementation and tests exist; first CI gate should test signatures, malformed/truncated input rejection, encoder/decoder round-trips, and benchmark reproducibility.
