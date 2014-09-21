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
#   For YV16, the "unique" argument to MakeTileWithDot sets the center
#   macropixel's values to 170 Y1, 85 Y2, 64 U, and 192 V. The tilesheet in use
#   has 256 shades of gray, each with an index matching its grayscale value, and
#   mode 4 for YV16 clips reads V to get the tile index.



Import("util.avs")

clip = MakeTileWithDot(16, 16, "unique", $FF8080, "YV16")
tilesheet = TurnsTileTestSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToYV16(matrix="PC.601")

TurnsTile(clip, tilesheet, mode=4)