#
# An exceptionally limited, simplistic tilesheet generator,
# used to create the sample sheets provided with the TurnsTile
# plugin package (http://www.gyroshot.com/turnstile.htm).
#
# Set your tile width and height with tilew and tileh, framerate
# using fps and fps_den, turn off individual channels with red,
# green and blue booleans, or set one or more of them to constant
# integer values with rconst, gconst and bconst.
#
# There isn't much you can do with this beyond what I already
# did to create the seven included PNGs, but I thought it
# might be of interest to someone. I'm still trying to get
# the script to work with tile sizes below 16x16, but so far
# I've only come agonizingly close. I'll probably figure it out
# one of these days, but don't hold your breath.
#
# If anyone has suggestions, of course, I'm all ears.

function TilesheetGen(	int "tilew", int "tileh", int "fps", int "fps_den", bool "red", bool "green", bool "blue",
\			int "rconst", int"gconst", int "bconst", string "pixel_type") {

	tilew = 	Default(tilew,	16)
	tileh =		Default(tileh,	16)
	fps =		Default(fps,	30000)
	fps_den =	Default(fps_den,1001)
	pixel_type =	Default(pixel_type, "RGB32")

	Assert(tilew >= 16, "TilesheetGen: tilew must be 16 or greater!")
	Assert(tileh >= 16, "TilesheetGen: tileh must be 16 or greater!")

	sheetw =	tilew * 16
	sheeth =	tileh * 16

	tilecountx =	sheetw / tilew
	tilecounty =	sheeth / tileh

	red =	Default(red, true)
	green =	Default(green, true)
	blue =	Default(blue, true)

	last = GetRow(tilew,tileh,tilecountx,Defined(rconst) ? rconst : 0,Defined(gconst) ? gconst : 0,Defined(bconst) ? bconst : 0,fps,fps_den,pixel_type)

	GScript("""

		for (i = 1, tilecounty - 1) {

			rowstartr = Defined(rconst) ? rconst : red == true ? i * tileh : 0
			rowstartg = Defined(gconst) ? gconst : green == true ? i * tileh : 0
			rowstartb = Defined(bconst) ? bconst : blue == true ? i * tileh : 0

			StackVertical(last,GetRow(tilew,tileh,tilecountx,rowstartr,rowstartg,rowstartb,fps,fps_den,pixel_type))

		}

	""")

}

function GetRow(int tilew, int tileh, int tilecountx, int rowstartr, int rowstartg, int rowstartb, int fps, int fps_den, string pixel_type) {

	last = GetTile(tilew,tileh,fps,fps_den,GetHexString(rowstartr,tilew,tileh,tilecountx),GetHexString(rowstartg,tilew,tileh,tilecountx),GetHexString(rowstartb,tilew,tileh,tilecountx),pixel_type)

	GScript("""
		for (j = 1, tilecountx - 1) {

			StackHorizontal(last,GetTile(tilew,tileh,fps,fps_den,GetHexString(rowstartr + j,tilew,tileh,tilecountx),GetHexString(rowstartg + j,tilew,tileh,tilecountx),GetHexString(rowstartb + j,tilew,tileh,tilecountx),pixel_type))

		}
	""")

}

function GetTile(int tilew, int tileh, int fps, int fps_den, string red, string green, string blue, string pixel_type) {

	Eval("BlankClip(width=tilew,height=tileh,fps=fps,fps_denominator=fps_den,pixel_type=pixel_type,color=$" + red + green + blue + ")")

}

function GetHexString(int num, int tilew, int tileh, int tilecountx) {

	leftidx = num / tileh
	leftidx = leftidx < 0 ? 0 : leftidx > 15 ? 15 : leftidx

	left =	Select(leftidx, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")
	right =	Select(num % tileh, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")

	return left + right

}