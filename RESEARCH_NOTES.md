# Prior art: vector and shape coding for images/video

**Search date:** 2026-11-16. This is a targeted first-pass literature map, not an exhaustive systematic review or full-text review. Google Scholar was queried through the Apify `johnvc/google-scholar-api` Actor with three one-page searches (two image-coding searches covering 1985–2010 and one video-coding search covering 1995–2010; at most five records each; each run had a $0.05 charge cap). Abstracts, search snippets, and linked publication pages were screened. Actual Actor charges were not returned.

## Direct image/contour coding

1. **“A contour-based image coding technique with its texture …” (1991).** The indexed abstract describes segmentation, contour extraction/encoding, and a separate texture stage. This is an early example of the hybrid idea: curves describe boundaries; texture still needs representation. [ScienceDirect record](https://www.sciencedirect.com/science/article/abs/pii/016516849190040P)
2. **“Application of image contours to three aspects of image processing: compression, shape recognition and stereopsis” (1992).** The abstract reports contour/vector coding and compression gains on some test images, not a universal result. [IET record](https://digital-library.theiet.org/doi/abs/10.1049/ip-i-2.1992.0001)
3. **“A vector coding method for natural images” (2000).** Represents pixel-value boundary contours on RGB/YCrCb planes with function approximation and predicts contours using similar ones. The authors describe scaling/editing as motivations. [IEEE record](https://ieeexplore.ieee.org/document/953617)
4. **“A vector coding with a compressive algorithm for natural images” (2000).** A related approach: approximate RGB boundary contours and predict shapes from similar contours at the decoder. [IEEE record](https://ieeexplore.ieee.org/document/911577)
5. **“Region-based coding of images using a spline model.”** Uses spline models for regions, combining region/function modeling with image coding. [IEEE record](https://ieeexplore.ieee.org/abstract/document/413723/)

## Video/object shape coding

6. **“MPEG-4 and rate-distortion-based shape-coding techniques.”** Reviews object-based coders and methods for coding object boundaries; shape is one coding problem within the video coder, not a complete Bézier-only video representation. [IEEE record](https://ieeexplore.ieee.org/document/687833) · [PDF copy](https://www.tnt.uni-hannover.de/papers/data/372/MPEG4andRateDistortionBasedShapeCoding.pdf)
7. **“Shape compression of moving objects using context-based arithmetic encoding.”** The indexed abstract describes exploiting spatial and temporal redundancy in object shapes as part of MPEG-4 video coding. [ScienceDirect record](https://www.sciencedirect.com/science/article/pii/S0923596599000478)
8. **“Coding of arbitrarily shaped video objects in MPEG-4.”** A standard-era treatment of shape coding for video objects. [IEEE record](https://ieeexplore.ieee.org/document/647958)
9. **“MPEG-4 natural video coding—An overview” (2000).** Describes MPEG-4’s object-oriented access/manipulation goals. [ScienceDirect record](https://www.sciencedirect.com/science/article/pii/S0923596599000545) · [PDF](http://www.img.lx.it.pt/~fp/cav/Additional_material/MPEG4_video.pdf)
10. **MPEG-4 object-based encoding overview (2001).** Discusses rate control and bit allocation across objects. [ACM record](https://dl.acm.org/doi/10.5555/876870.878137)

## Established vector description

11. **W3C SVG 1.0 (2001).** A standard for 2D vector and mixed vector/raster graphics. Its existence disproves “no one did vector graphics”; it is not, by itself, a codec for general camera photos or video. [W3C specification](https://www.w3.org/TR/2001/REC-SVG-20010904/)

## What the prior art says—and does not say

- **Not new:** representing contours, regions, and shapes with curves; image contour coding; object/shape coding for video; user-selected rate/distortion tradeoffs.
- **Not established by these records:** a general vector-only image/video format that beats modern raster codecs at matched perceptual quality, or VectorVision’s advertised compression ratio. Abstracts/snippets are not enough to settle those claims.
- **Likely engineering barriers, supported by the shape/texture literature:** meaningful segmentation is difficult; contour coding does not encode texture automatically; moving boundaries require temporal prediction and handling of occlusion/topology changes. These explain why prior systems often paired shape with texture and did not replace ordinary rectangular-frame codecs.

## Follow-up reading order

Read the full 1991 contour+texture paper, both 2000 natural-image vector-coding papers, then the MPEG-4 shape-coding survey and natural-video overview. Record their datasets, rate-distortion curves, texture treatment, decoder complexity, and failure cases before making novelty or performance claims.
