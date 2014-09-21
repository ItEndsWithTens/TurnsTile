# TurnsTile - Lotile and hitile options produce expected result
# [output][turnstile][lotile][hitile]
#
# Expected:
#
#   64x16 gray frame, with the following RGB component values for each quarter:
#     First: 64 64 64
#     Second: 96 96 96
#     Third: 160 160 160
#     Fourth: 192 192 192
#
# Rationale:
#
#   The lotile and hitile parameters, in tilesheet mode, scale the range of
#   permissible tile indices to lotile through hitile, inclusive. This test clip
#   has four adjacent 16x16 tiles, with RGB values of 0, 64, 192, and 255.
#
#   This input clip has only four input values, with the second being above the
#   first, and the third below the fourth, by twenty-five percent of the total
#   range size. Specifying a range of 64 through 192 gives an output range size
#   of one hundred twenty-eight. Twenty-five percent of that is thirty-two, which
#   means the second and third indices should be 96 and 160. The loaded tilesheet
#   has two hundred fifty-six tiles, and each tile's components are set to the
#   same value as that tile's index.
#
#   As with the clip version of this script, this is only tested in RGB since the
#   relevant logic is mostly colorspace independent, and what isn't is already
#   covered in other test scripts.



Import("util.avs")

a = BlankClip(width=16, height=16, pixel_type="RGB24", color=$000000)
b = BlankClip(width=16, height=16, pixel_type="RGB24", color=$404040)
c = BlankClip(width=16, height=16, pixel_type="RGB24", color=$C0C0C0)
d = BlankClip(width=16, height=16, pixel_type="RGB24", color=$FFFFFF)
clip = StackHorizontal(a, b, c, d)

tilesheet = TurnsTileTestSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToRGB24()

TurnsTile(clip, tilesheet, lotile=64, hitile=192)