# TurnsTile - Interlaced option produces expected result
# [output][turnstile][interlaced]
#
# Expected:
#
#   640x240 frame, with the following RGB component values for each half:
#     First: 255 0 0
#     Second: 255 255 0
#
# Rationale:
#
#   The interlaced parameter is implemented within TurnsTile as a simple pairing
#   of SeparateFields before processing and Weave afterward. When interlaced is
#   set to true, the tile effect is applied to each field independently. In this
#   case, with solid color fields and using a tilesheet with solid colors, the
#   black field should become the lowest numbered tile (zero, in this case red),
#   and the white field should become the highest numbered (five, or yellow). If
#   interlaced were set to false, both would turn red, since the effect would be
#   applied across both sets of scanlines, and for each tile a pixel from the
#   black field would be chosen as the tile index.



Import("util.avs")
InitializeTurnsTileTestEnvironment()

a = BlankClip(width=320, color=$000000).SeparateFields().SelectEven()
b = BlankClip(width=320, color=$FFFFFF).SeparateFields().SelectOdd()

clip = Interleave(a, b).Weave()
tilesheet = MakeTilesheet(16, 16, "RGB32")
result = TurnsTile(clip, tilesheet, interlaced=true)

result_even = result.SeparateFields().SelectEven()
result_odd = result.SeparateFields().SelectOdd()
StackHorizontal(result_even, result_odd)