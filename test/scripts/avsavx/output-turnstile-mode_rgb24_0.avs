# TurnsTile - Mode option produces expected result
# [output][turnstile][mode]
#
# Expected:
#
#   16x16 gray frame, with the following component values for each pixel:
#     RGB: 170 170 170
#
# Rationale:
#
#   For RGB24, the "unique" argument to MakeTileWithDot sets the center pixel's
#   values to 255 red, 170 green, and 85 blue. The tilesheet in use has 256
#   shades of gray, each with an index matching its grayscale value, and mode 0
#   for RGB24 clips averages all three components to get the tile index.



Import("util.avs")

clip = MakeTileWithDot(16, 16, "unique", $FFFFFF, "RGB24")
tilesheet = TurnsTileTestSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToRGB24()

TurnsTile(clip, tilesheet, mode=0)