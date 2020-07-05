# TurnsTile - Levels option produces expected result
# [output][turnstile][levels]
#
# Expected:
#
#   Black and white frame, black on bottom and white on top.
#
# Rationale:
#
#   The frame is divided into four horizontal bands, with average values of 255,
#   235, 16, and 0 from top to bottom. With levels set to "tv", the 235 band
#   should use the highest numbered tile just as the 255 band does, which is
#   white, while the 16 and 0 bands both use the lowest number, which is black.



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

clip = TurnsTileTestSource("../clips/output-turnstile-levels-clip.png")
clip = clip.ConvertToY8(matrix="PC.601")
tilesheet = TurnsTileTestSource("../clips/output-turnstile-levels-tilesheet.png")
tilesheet = tilesheet.ConvertToY8(matrix="PC.601")

TurnsTile(clip, tilesheet, levels="tv")