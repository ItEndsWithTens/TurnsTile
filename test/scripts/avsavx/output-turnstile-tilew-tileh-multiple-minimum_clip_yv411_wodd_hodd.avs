# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   36x9 white frame with a 12x3 black tile in each corner.
#
# Rationale:
#
#   YV411 with odd multiple width and odd multiple height should show no bias:
#
#     lumaW = 4
#     lumaH = 1
#
#     tilew == 12
#       tileCtrW_Y == 12 / 2 == 6 == floor(6 / lumaW) * lumaW == 4
#       tileCtrW_U == (12 / lumaW) / 2 == floor(1.5) == 1
#
#     tileh == 3
#       tileCtrH_Y == 3 / 2 == floor(1.5) == 1
#       tileCtrH_U == (3 / lumaH) / 2 == floor(1.5) == 1
#
#     Luma                        Chroma
#               V                     V
#      +-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+
#      |0|1|2|3|4|5|6|7|8|9|A|B|   |0|1|2|
#      +-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+
#     >|1| | | |x|x|x|x| | | | |  >|1|x| |
#      +-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+
#      |2| | | | | | | | | | | |   |2| | |
#      +-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+



function GetScriptDirectory()
{

  try {

    Assert(false)

  } catch(err_msg) {

    err_msg = MidStr(err_msg, FindStr(err_msg, "(") + 1)
    script = LeftStr(err_msg, StrLen(err_msg) - FindStr(RevStr(err_msg), ","))

  }

  rev = RevStr(script)
  bk_pos = FindStr(rev, "\")
  fw_pos = FindStr(rev, "/")
  bk_pos = bk_pos > 0 ? bk_pos : StrLen(rev)
  fw_pos = fw_pos > 0 ? fw_pos : StrLen(rev)

  sep_pos = bk_pos < fw_pos ? bk_pos : fw_pos

  return LeftStr(script, StrLen(script) - sep_pos)

}



SetWorkingDir(GetScriptDirectory())
Import("util.avs")


pixel_type = "YV411"
tilew = GetMinWidth(pixel_type) * 3
tileh = GetMinHeight(pixel_type) * 3

MakeClip(tilew * 3, tileh * 3, $FF8080, $008080, $558080, pixel_type)
TurnsTile(tilew=tilew, tileh=tileh)