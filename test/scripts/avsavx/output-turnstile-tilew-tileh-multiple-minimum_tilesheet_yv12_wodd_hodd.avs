# TurnsTile - Tile dimension at multiples of minimum produces expected result
# [output][turnstile][tilew][tileh][multiple][minimum]
#
# Expected:
#
#   18x18 yellow frame with a 6x6 red tile in each corner.
#
# Rationale:
#
#   Black sample at center of tile corresponds to lowest numbered tile in
#   tilesheet, which is red, while white is the highest numbered, yellow.



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


pixel_type = "YV12"
tilew = GetMinWidth(pixel_type) * 3
tileh = GetMinHeight(pixel_type) * 3

clip = MakeClip(tilew * 3, tileh * 3, $FF8080, $008080, $558080, pixel_type)
tilesheet = MakeTilesheet(tilew, tileh, pixel_type)

TurnsTile(clip, tilesheet, tilew=tilew, tileh=tileh)