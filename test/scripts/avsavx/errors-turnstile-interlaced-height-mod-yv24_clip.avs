Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="YV24")

TurnsTile(clip, interlaced=true)