# TurnsTile - Mode option produces expected result
# [output][turnstile][mode]
#
# Expected:
#
#   16x16 gray frame, with the following component values for each pixel:
#     RGBA: 170 170 170 255
#
# Rationale:
#
#   For RGB32, the "unique" argument to MakeTileWithDot sets the center pixel's
#   values to 255 red, 170 green, 85 blue, and 64 alpha. The tilesheet in use has
#   256 shades of gray, each with an index matching its grayscale value, and mode
#   0 for RGB32 clips averages the red, green, and blue components to get the
#   tile index. TurnsTile additionally always sets output alpha to 255.



Import("util.avs")

clip = MakeTileWithDot(16, 16, "unique", $FFFFFF, "RGB32")
tilesheet = TurnsTileTestSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToRGB32()

TurnsTile(clip, tilesheet, mode=0)