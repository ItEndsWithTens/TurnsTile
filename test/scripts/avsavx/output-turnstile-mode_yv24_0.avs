# TurnsTile - Mode option produces expected result
# [output][turnstile][mode]
#
# Expected:
#
#   16x16 gray frame, with the following component values for each pixel:
#     YUV: 170 128 128
#
# Rationale:
#
#   For YV24, the "unique" argument to MakeTileWithDot sets the center
#   macropixel's values to 170 Y, 64 U, and 192 V. The tilesheet in use has 256
#   shades of gray, each with an index matching its grayscale value, and mode 0
#   for YV24 clips reads Y to get the tile index.



Import("util.avs")

clip = MakeTileWithDot(16, 16, "unique", $FF8080, "YV24")
tilesheet = ImageSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToYV24(matrix="PC.601")

TurnsTile(clip, tilesheet, mode=0)