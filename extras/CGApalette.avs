####
#
# A sample palette, to be used for the AviSynth plugin "CLUTer", by way of AVISource.
# Based on CGA colors, as described at http://en.wikipedia.org/wiki/Color_Graphics_Adapter
#
# The plugin uses Euclidean Distance as its color matching technique, and it isn't
# a perfect representation of human vision. Using this palette, with several varieties
# of gray, you may find your results rather lacking in vibrance. Try using Tweak(sat=2.5)
# for a start, use higher saturation values if necessary, and if worse comes to worst, modify
# this file to remove a few of those gray entries from the palette. Or just use another
# one, for that matter; this is only a demonstration.
#
# CLUTer download, as part of the TurnsTile plugin: http://www.gyroshot.com/turnstile.htm
#
# robert.martens@gmail.com
#
####

csp = "RGB32"
width = 16
height = 16

black =		BlankClip(width=width,height=height,pixel_type=csp,color=$000000)
blue =		BlankClip(width=width,height=height,pixel_type=csp,color=$0000AA)
green =		BlankClip(width=width,height=height,pixel_type=csp,color=$00AA00)
cyan =		BlankClip(width=width,height=height,pixel_type=csp,color=$00AAAA)
red =		BlankClip(width=width,height=height,pixel_type=csp,color=$AA0000)
magenta =	BlankClip(width=width,height=height,pixel_type=csp,color=$AA00AA)

# According to the Wikipedia page, brown is the one of these two options that's
# more frequently encountered, and that's carried on from CGA by EGA and VGA,
# so that's my default. Use dark yellow if you prefer.

colorsix =	BlankClip(width=width,height=height,pixel_type=csp,color=$AA5500) # Brown
#colorsix =	BlankClip(width=width,height=height,pixel_type=csp,color=$AAAA00) # Dark yellow

lightgray =	BlankClip(width=width,height=height,pixel_type=csp,color=$AAAAAA)
gray =		BlankClip(width=width,height=height,pixel_type=csp,color=$555555)
lightblue =	BlankClip(width=width,height=height,pixel_type=csp,color=$5555FF)
lightgreen =	BlankClip(width=width,height=height,pixel_type=csp,color=$55FF55)
lightcyan =	BlankClip(width=width,height=height,pixel_type=csp,color=$55FFFF)
lightred =	BlankClip(width=width,height=height,pixel_type=csp,color=$FF5555)
lightmagenta =	BlankClip(width=width,height=height,pixel_type=csp,color=$FF55FF)
yellow =	BlankClip(width=width,height=height,pixel_type=csp,color=$FFFF55)
white =		BlankClip(width=width,height=height,pixel_type=csp,color=$FFFFFF)

top =		StackHorizontal(black,blue,green,cyan,red,magenta,colorsix,lightgray)
bottom =	StackHorizontal(gray,lightblue,lightgreen,lightcyan,lightred,lightmagenta,yellow,white)

StackVertical(top,bottom)