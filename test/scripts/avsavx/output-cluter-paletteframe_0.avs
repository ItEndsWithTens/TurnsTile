# CLUTer - Paletteframe option produces expected result
# [output][cluter][paletteframe]
#
# Expected:
#
#   512x512 clip with four adjacent vertical bands of color: first red, then
#   green, blue, and red again.
#
# Rationale:
#
#   The input clip is a 512x512 plot of the HSL colorspace, and the input palette
#   clip features alternating frames of RGB and CMY. Since paletteframe is set to
#   0, the first frame (red, green, and blue) is selected as the palette.



Import("util.avs")

clip = TurnsTileTestSource("../clips/hsl.png", "RGB24")

palette_base = BlankClip(width=16, height=16, pixel_type="RGB24")

r = BlankClip(palette_base, color=$FF0000)
g = BlankClip(palette_base, color=$00FF00)
b = BlankClip(palette_base, color=$0000FF)
palette_a = StackHorizontal(r, g, b)

c = BlankClip(palette_base, color=$00FFFF)
m = BlankClip(palette_base, color=$FF00FF)
y = BlankClip(palette_base, color=$FFFF00)
palette_b = StackHorizontal(c, m, y)

palette = Interleave(palette_a, palette_b)

CLUTer(clip, palette, 0)