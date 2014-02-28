# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   12x9 white frame with a 4x3 black tile in each corner.
#
# Rationale:
#
#   Y8 with even multiple width and odd multiple height should favor right
#   center:
#
#     lumaW = 1
#     lumaH = 1
#
#     tilew == 4
#       tileCtrW_Y == 4 / 2 == 2
#
#     tileh == 3
#       tileCtrH_Y == 3 / 2 == floor(1.5) == 1
#
#     Luma
#
#           V
#      +-+-+-+-+
#      |0|1|2|3|
#      +-+-+-+-+
#     >|1| |x| |
#      +-+-+-+-+
#      |2| | | |
#      +-+-+-+-+



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


pixel_type = "Y8"
tilew = GetMinWidth(pixel_type) * 4
tileh = GetMinHeight(pixel_type) * 3

MakeClip(tilew * 3, tileh * 3, $FF8080, $008080, $558080, pixel_type)

TurnsTile(tilew=tilew, tileh=tileh)