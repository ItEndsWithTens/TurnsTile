# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   9x9 white frame with a 3x3 black tile in each corner.
#
# Rationale:
#
#   Y8 with odd multiple width and odd multiple height should show no bias:
#
#     lumaW = 1
#     lumaH = 1
#
#     tilew == 3
#       tileCtrW_Y == 3 / 2 == floor(1.5) == 1
#       tileCtrW_U == 3 / 2 == floor(1.5) == 1
#
#     tileh == 3
#       tileCtrH_Y == 3 / 2 == floor(1.5) == 1
#       tileCtrH_U == 3 / 2 == floor(1.5) == 1
#
#     Luma    
#             
#         V   
#      +-+-+-+
#      |0|1|2|
#      +-+-+-+
#     >|1|x| |
#      +-+-+-+
#      |2| | |
#      +-+-+-+



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
tilew = GetMinWidth(pixel_type) * 3
tileh = GetMinHeight(pixel_type) * 3

MakeClip(tilew * 3, tileh * 3, $FFFFFF, $000000, $555555, pixel_type)

TurnsTile(tilew=tilew, tileh=tileh)