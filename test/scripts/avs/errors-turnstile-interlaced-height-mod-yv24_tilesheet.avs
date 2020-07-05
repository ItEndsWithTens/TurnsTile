Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV24")
tilesheet = BlankClip(height=479, pixel_type="YV24")

TurnsTile(clip, tilesheet, interlaced=true)