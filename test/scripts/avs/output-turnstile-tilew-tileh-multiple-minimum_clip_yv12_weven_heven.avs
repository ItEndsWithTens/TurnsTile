# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   24x24 white frame with an 8x8 black tile in each corner.
#
# Rationale:
#
#   YV12 with even multiple width and even multiple height should favor right
#   bottom:
#
#     lumaW = 2
#     lumaH = 2
#
#     tilew == 8
#       tileCtrW_Y == 8 / 2 == 4
#       tileCtrW_U == (8 / lumaW) / 2 == 2
#
#     tileh == 8
#       tileCtrH_Y == 8 / 2 == 4
#       tileCtrH_U == (8 / lumaH) / 2 == 2
#
#     Luma                Chroma
#               V               V
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |0|1|2|3|4|5|6|7|   |0|1|2|3|
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |1| | | | | | | |   |1| | | |
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |2| | | | | | | |  >|2| |x| |
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |3| | | | | | | |   |3| | | |
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#     >|4| | | |x|x| | |
#      +-+-+-+-+-+-+-+-+
#      |5| | | |x|x| | |
#      +-+-+-+-+-+-+-+-+
#      |6| | | | | | | |
#      +-+-+-+-+-+-+-+-+
#      |7| | | | | | | |
#      +-+-+-+-+-+-+-+-+



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
InitializeTurnsTileTestEnvironment()


pixel_type = "YV12"
tilew = GetMinWidth(pixel_type) * 4
tileh = GetMinHeight(pixel_type) * 4

MakeClip(tilew * 3, tileh * 3, $FF8080, $008080, $558080, pixel_type)

TurnsTile(tilew=tilew, tileh=tileh)