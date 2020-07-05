Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="RGB24")
tilesheet = BlankClip(height=479, pixel_type="RGB24")

TurnsTile(clip, tilesheet, interlaced=true)