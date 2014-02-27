# TurnsTile - Interlaced option produces expected result
# [output][turnstile][interlaced]
#
# Expected:
#
#   640x240 frame, with the following RGB component values for each half:
#     First: 255 0 0
#     Second: 0 255 0
#
# Rationale:
#
#   The interlaced parameter is implemented within TurnsTile as a simple pairing
#   of SeparateFields before processing and Weave afterward. When interlaced is
#   set to true, the tile effect is applied to each field independently. In this
#   case, with solid color fields and not using a tilesheet, each field should
#   remain its input color. If interlaced were set to false, both would turn red,
#   since the effect would be applied across both sets of scanlines, and for each
#   tile a pixel from the red field would be chosen as the output value.



a = BlankClip(width=320, color=$FF0000).SeparateFields().SelectEven()
b = BlankClip(width=320, color=$00FF00).SeparateFields().SelectOdd()

Interleave(a, b)
Weave()

result = TurnsTile(interlaced=true)

result_even = result.SeparateFields().SelectEven()
result_odd = result.SeparateFields().SelectOdd()
StackHorizontal(result_even, result_odd)