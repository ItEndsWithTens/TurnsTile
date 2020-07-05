# TurnsTile - Mode option produces expected result
# [output][turnstile][mode]
#
# Expected:
#
#   16x16 gray frame, with the following component values for each pixel:
#     Y: 170
#
# Rationale:
#
#   For Y8, the "unique" argument to MakeTileWithDot sets the center
#   macropixel's value to 170 Y. The tilesheet in use has 256 shades of gray,
#   each with an index matching its grayscale value, and mode 0 for Y8 clips
#   reads Y to get the tile index.



Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = MakeTileWithDot(16, 16, "unique", $FF8080, "Y8")
tilesheet = TurnsTileTestSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToY8(matrix="PC.601")
TurnsTile(clip, tilesheet, mode=0)