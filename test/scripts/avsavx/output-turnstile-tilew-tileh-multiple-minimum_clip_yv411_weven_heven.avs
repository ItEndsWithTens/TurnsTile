# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   48x12 white frame with a 16x4 black tile in each corner.
#
# Rationale:
#
#   YV411 with even multiple width and even multiple height should favor right
#   bottom:
#
#     lumaW = 4
#     lumaH = 1
#
#     tilew == 16
#       tileCtrW_Y == 16 / 2 == 8
#       tileCtrW_U == (16 / lumaW) / 2 == 2
#
#     tileh == 4
#       tileCtrH_Y == 4 / 2 == 2
#       tileCtrH_U == (4 / lumaH) / 2 == 2
#
#     Luma                                Chroma
#                       V                       V
#      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|   |0|1|2|3|
#      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |1| | | | | | | | | | | | | | | |   |1| | | |
#      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+-+
#     >|2| | | | | | | |x|x|x|x| | | | |  >|2| |x| |
#      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |3| | | | | | | | | | | | | | | |   |3| | | |
#      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   +-+-+-+-+



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
tilew = GetMinWidth(pixel_type) * 4
tileh = GetMinHeight(pixel_type) * 4

MakeClip(tilew * 3, tileh * 3, $FF8080, $008080, $558080, pixel_type)
TurnsTile(tilew=tilew, tileh=tileh)