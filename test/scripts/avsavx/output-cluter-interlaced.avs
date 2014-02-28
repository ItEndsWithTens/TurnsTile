# CLUTer - Interlaced option produces expected result
# [output][cluter][interlaced]
#
# Expected:
#
#   64x64 green and red frame, which when viewed assuming interlaced YV12 chroma
#   will show alternating lines of green and red, with consistent color across
#   their entire width.
#
# Rationale:
#
#   There are two 32x64 clips stacked horizontally here, the original clip on the
#   left and the CLUTer result on the right. The palette in use features three
#   colors, two of which are identical to the two colors used in the input clip,
#   so if the palette selection and interlaced processing are working correctly
#   the output color should match the input.
#
#   Set interlaced to false, and view the result as progressive YV12 chroma, to
#   demonstrate that CLUTer applies the same luma and chroma to every 2x2 YV12
#   macropixel, producing two-line high bands across the right half of the image.



palette_base = BlankClip(width=16, height=16, pixel_type="YV12")
r = BlankClip(palette_base, color_yuv=$4C55FF)
g = BlankClip(palette_base, color_yuv=$962B15)
b = BlankClip(palette_base, color_yuv=$1DFF6B)
palette = StackHorizontal(r, g, b)

clip_base = BlankClip(width=32, height=32, pixel_type="YV12")
lower = BlankClip(clip_base, color_yuv=$4C55FF).AssumeFieldBased()
upper = BlankClip(clip_base, color_yuv=$962B15).AssumeFieldBased()
clip = Interleave(lower, upper).Weave()

result = CLUTer(clip, palette, interlaced=true)

return StackHorizontal(clip, result)