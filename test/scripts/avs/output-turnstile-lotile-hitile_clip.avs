# TurnsTile - Lotile and hitile options produce expected result
# [output][turnstile][lotile][hitile]
#
# Expected:
#
#   64x16 gray frame, with the following RGB component values for each half:
#     First: 64 64 64
#     Second: 192 192 192
#
# Rationale:
#
#   The lotile and hitile parameters, in clip mode, clamp the minimum and
#   maximum component values available for output.  This test clip has four
#   adjacent 16x16 tiles, with RGB values of 0, 64, 192, and 255.
#
#   For clip mode I only test these parameters in RGB24, since most of the logic
#   that might go wrong is colorspace independent, and other tests (namely those
#   for the res parameter) already cover alterations to the sample lookup table.



Import("util.avs")
InitializeTurnsTileTestEnvironment()

a = BlankClip(width=16, height=16, pixel_type="RGB24", color=$000000)
b = BlankClip(width=16, height=16, pixel_type="RGB24", color=$404040)
c = BlankClip(width=16, height=16, pixel_type="RGB24", color=$C0C0C0)
d = BlankClip(width=16, height=16, pixel_type="RGB24", color=$FFFFFF)
StackHorizontal(a, b, c, d)

TurnsTile(lotile=64, hitile=192)