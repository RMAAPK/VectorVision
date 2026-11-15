# VectorVision media formats — design record

**Status:** draft. The signatures below are approved; this document does not yet define a complete codec or payload.

## File identity

The signature is the first eight bytes of the file, at offset 0. Files must not have a BOM or prefix before it.

| Extension | Meaning | Bytes (escaped) | Hex |
|---|---|---|---|
| `.svgc` | One still-image scene | `SVGC\r\n\x1A\n` | `53 56 47 43 0D 0A 1A 0A` |
| `.svgv` | A timed sequence of scenes | `SVGV\r\n\x1A\n` | `53 56 47 56 0D 0A 1A 0A` |

## Version 1.0 wire header

A packed 60-byte header is followed by one zlib/DEFLATE payload. All multi-byte fields are little-endian. CRC uses IEEE 802.3 CRC-32 (reflected polynomial `0xEDB88320`, initial/final XOR `0xFFFFFFFF`). Header CRC covers bytes `[0,56)`; payload CRC covers the compressed payload.

| Offset | Field | Encoding |
|---:|---|---|
| 0 | magic | 8 bytes from table above |
| 8, 10 | major, minor | `u16`, version `1.0` |
| 12 | header_bytes | `u16`, exactly `60` |
| 14, 15 | quality, flags | quality `0..100`; flags `0` |
| 16, 18 | profile, reserved | `u16`; profile `1`, reserved `0` |
| 20, 24 | width, height | `u32` pixels; each `1..16384` |
| 28, 32 | timebase numerator, denominator | `u32`; both `0` for SVGC; seconds per tick for SVGV |
| 36, 44 | payload_bytes, raw_payload_bytes | `u64` compressed and uncompressed lengths |
| 52, 56 | payload CRC, header CRC | `u32` IEEE CRC-32 |

Limits: each payload length is at most 256 MiB. Decoders reject unknown versions/flags/profiles, length mismatches, invalid CRCs, and truncated data.

## Payload

The uncompressed SVGC payload is one scene: `u32 path_count`, then for each path RGBA bytes, a `u8 closed` flag, `u32 curve_count`, a signed 16.16 start point, then `curve_count` cubic Béziers (six signed 16.16 coordinates per curve: two controls and endpoint). Integers are little-endian.

The uncompressed SVGV payload starts with `u32 frame_count`; each frame is `u64 timestamp_ticks`, `u64 scene_payload_bytes`, then the same scene payload. Timestamps strictly increase. V1 stores full vector scenes per frame; temporal delta coding is not implemented yet. The experimental residual is benchmark-only and is not part of either bitstream.

## Quality semantics

The header quality byte records the encoder's requested target (`0` lowest through `100` highest); profile `1` identifies the path encoding, not a calibrated quality mapping. The decoder renders the geometry actually stored and does not reinterpret the slider. A high setting is not lossless unless the source is exactly preserved. The synthetic fixture varies geometric detail by quality for rate-distortion testing; a general raster-to-curve encoder and calibrated mapping remain future work.

The signatures identify file type and versioned payload framing; compression results and visual fidelity require benchmark evidence. The quality profile and payload may evolve only under a new compatible version/profile decision.

## Decision log

- 2026-11-16: Project owner approved the two eight-byte signatures above.
