# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   18x9 white frame with a 6x3 black tile in each corner.
#
# Rationale:
#
#   YV16 with odd multiple width and odd multiple height should show no bias:
#  
#     lumaW = 2
#     lumaH = 1
#
#     tilew == 6
#       tileCtrW_Y == 6 / 2 == floor(3 / lumaW) * lumaW == 2
#       tileCtrW_U == (6 / lumaW) / 2 == floor(1.5) == 1
#
#     tileh == 3
#       tileCtrH_Y == 3 / 2 == floor(1.5) == 1
#       tileCtrH_U == (3 / lumaH) / 2 == floor(1.5) == 1
#
#     Luma            Chroma
#           V             V
#      +-+-+-+-+-+-+   +-+-+-+
#      |0|1|2|3|4|5|   |0|1|2|
#      +-+-+-+-+-+-+   +-+-+-+
#     >|1| |x|x| | |  >|1|x| |
#      +-+-+-+-+-+-+   +-+-+-+
#      |2| | | | | |   |2| | |
#      +-+-+-+-+-+-+   +-+-+-+



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


pixel_type = "YV16"
tilew = GetMinWidth(pixel_type) * 3
tileh = GetMinHeight(pixel_type) * 3

MakeClip(tilew * 3, tileh * 3, $FF8080, $008080, $558080, pixel_type)

TurnsTile(tilew=tilew, tileh=tileh)