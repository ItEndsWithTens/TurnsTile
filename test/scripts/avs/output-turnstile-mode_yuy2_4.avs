# TurnsTile - Mode option produces expected result
# [output][turnstile][mode]
#
# Expected:
#
#   16x16 gray frame, with the following component values for each pixel:
#     YUV: 192 128 128
#
# Rationale:
#
#   For YUY2, the "unique" argument to MakeTileWithDot sets the center
#   macropixel's values to 170 Y1, 64 U, 85 Y2, and 192 V. The tilesheet in use
#   has 256 shades of gray, each with an index matching its grayscale value, and
#   mode 4 for YUY2 clips reads V to get the tile index.



Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = MakeTileWithDot(16, 16, "unique", $FF8080, "YUY2")
tilesheet = TurnsTileTestSource("../clips/output-turnstile-levels-tilesheet-yuy2.ebmp")

TurnsTile(clip, tilesheet, mode=4)