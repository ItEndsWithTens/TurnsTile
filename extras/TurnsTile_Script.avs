#
#	Script implementation of TurnsTile 0.1.0
#
# Requires GScript: http://forum.doom9.org/showthread.php?t=147846
# and GRunT: http://forum.doom9.org/showthread.php?t=139337
#
# Just to be clear, this script is not intended for actual use; it's merely a demonstration of the
# difference I experienced between the script and plugin versions of the same function. The results
# aren't quite identical, but are close enough to make the point.
#
# If you somehow ended up with a copy of this file, but not the rest of the archive, please visit
# http://www.gyroshot.com/turnstile.htm to download the full plugin package.

function TurnsTile_Script(clip c, clip tilesheet, int "tilew", int "tileh", int "res") {

	# In a desperate attempt to get this to run faster, I set up as many
	# variables as possible ahead of time. Most won't be needed at this
	# level, but doing all of this here means they won't be recalculated
	# on every iteration of every loop in the script. The function calls
	# do become rather long as a result, but it's still preferable to the
	# drop in speed brought on by the alternative.

	tilew =		Default(tilew,	16)
	tileh =		Default(tileh,	16)

	sheettilesx =	tilesheet.Width() / tilew
	sheettilesy =	tilesheet.Height() / tileh
	sheettiles =	sheettilesx * sheettilesy

	comp =		BlankClip(c,width=tilew,height=tileh)	# Something to compare against for RGBDifference; there's
								# no RGBAverage() function, so this does the job for us.

	res =		Default(res,	16)
	diffstring =	c.IsRGB() ? "RGBDifference(last,comp)" : "AverageLuma(last)"

	cropx =		0
	cropy =		0

	# I first need to build the top row, outside of the outer loop, so
	# that StackVertical has something to stack against. On subsequent
	# loops, 'last' will contain the result of the Stack. This could
	# probably also be achieved using a recursive function call, but
	# I think GScript makes things a little neater.

	last =		TTS_BuildRow(	c,tilesheet,comp,tilew,tileh,sheettilesx,sheettilesy,sheettiles,cropx,cropy,
\					sheettiles - 1,res,diffstring)

	GScript("""

		for (cropy = 1, c.Height() / tileh - 1) {

			StackVertical(last,TTS_BuildRow(c,tilesheet,comp,tilew,tileh,sheettilesx,sheettilesy,sheettiles,
\							cropx,cropy * tileh,sheettiles - 1,res,diffstring))

		}

	""")

}

function TTS_BuildRow(	clip c, clip tilesheet, clip comp, int tilew, int tileh, int sheettilesx,
\			int sheettilesy, int sheettiles, int cropx, int cropy, int scalemax,
\			int res, string diffstring) {

	# Just as with the outer loop, the inner one needs something to
	# stack against, so this grabs the first tile before starting
	# the loop across the current row.

	last = TTS_TileGrab(	c,tilesheet,comp,tilew,tileh,sheettilesx,sheettilesy,sheettiles,cropx,cropy,
\				scalemax,res,diffstring)

	GScript("""

		for (cropx = 1, c.Width() / tilew - 1) {

			StackHorizontal(last,TTS_TileGrab(	c,tilesheet,comp,tilew,tileh,sheettilesx,sheettilesy,sheettiles,
\								cropx * tilew,cropy,scalemax,res,diffstring))

		}
	""")

}

function TTS_TileGrab(	clip c, clip tilesheet, clip comp, int tilew, int tileh, int sheettilesx,
\			int sheettilesy, int sheettiles, int cropx, int cropy, int scalemax,
\			int res, string diffstring) {

	# Doing the tile crop out here means it happens once for every
	# tile in the image, but not every frame; the only things that
	# happen per frame are grabbing the tile index, scaling the value
	# to account for the number of tilesheet tiles, and cropping the sheet
	# appropriately. It's still a lot to ask of a script, though,
	# and try as I might I couldn't get this idea to run any faster
	# without writing a full on plugin.

	tile =	c.Crop(cropx,cropy,tilew,tileh)

	GScriptClip(tile,"""

		Eval(" tileidx = " + diffstring + ".Round().TTS_ScaleToRange(scalemax,res) ")

		tilesheet.Crop((tileidx % sheettilesx) * tilew,tileidx / sheettilesx * tileh,tilew,tileh)


	""",args="tilesheet,comp,tilew,tileh,sheettilesx,sheettilesy,sheettiles,scalemax,res,diffstring")

}

function TTS_ScaleToRange(int num, int outmax, int res) {

	# A full description of this scaling formula is in the source
	# code for the plugin, see TurnsTile.cpp for details. The
	# second step here, of rounding to res, is complicated by the
	# use of integer arithmetic, which always rounds toward zero.
	# Adding half of res first provides the effect of the rounding
	# most of us learned in grade school ( >= .5 rounds up, < .5
	# rounds down). If a number were negative, you'd instead need
	# to subtract half of res, but since these values are always
	# positive during my work here, it's not a concern.

	result = num / (255 / outmax)

	result = (result + (res / 2)) / res * res

	return result > outmax ? outmax : result

}