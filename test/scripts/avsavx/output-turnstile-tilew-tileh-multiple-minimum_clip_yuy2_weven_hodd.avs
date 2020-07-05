# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   24x9 white frame with an 8x3 black tile in each corner.
#
# Rationale:
#
#   YUY2 with even multiple width and odd multiple height should favor right
#   center:
#
#     lumaW = 2
#     lumaH = 1
#
#     tilew == 8
#       tileCtrW_Y == 8 / 2 == 4
#       tileCtrW_U == (8 / lumaW) / 2 == 2
#
#     tileh == 3
#       tileCtrH_Y == 3 / 2 == floor(1.5) == 1
#       tileCtrH_U == (3 / lumaH) / 2 == floor(1.5) == 1
#
#     Luma                Chroma
#               V               V
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |0|1|2|3|4|5|6|7|   |0|1|2|3|
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#     >|1| | | |x|x| | |  >|1| |x| |
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+
#      |2| | | | | | | |   |2| | | |
#      +-+-+-+-+-+-+-+-+   +-+-+-+-+



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


pixel_type = "YUY2"
tilew = GetMinWidth(pixel_type) * 4
tileh = GetMinHeight(pixel_type) * 3

MakeClip(tilew * 3, tileh * 3, $FF8080, $008080, $558080, pixel_type)

TurnsTile(tilew=tilew, tileh=tileh)