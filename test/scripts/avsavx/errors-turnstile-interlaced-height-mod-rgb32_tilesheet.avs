Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="RGB32")
tilesheet = BlankClip(height=479, pixel_type="RGB32")

TurnsTile(clip, tilesheet, interlaced=true)