# TurnsTile - Mode option produces expected result
# [output][turnstile][mode]
#
# Expected:
#
#   16x16 gray frame, with the following component values for each pixel:
#     YUV: 0 128 128
#
# Rationale:
#
#   For YV411, the "unique" argument to MakeTileWithDot sets the center
#   macropixel's values to 255 Y1, 170 Y2, 85 Y3, 0 Y4, 64 U, and 192 V. The
#   tilesheet in use has 256 shades of gray, each with an index matching its
#   grayscale value, and mode 4 for YV411 clips reads Y4 to get the tile index.



Import("util.avs")

clip = MakeTileWithDot(16, 16, "unique", $FF8080, "YV411")
tilesheet = ImageSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToYV411(matrix="PC.601")

TurnsTile(clip, tilesheet, mode=4)