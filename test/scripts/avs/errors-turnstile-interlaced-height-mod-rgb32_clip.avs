Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="RGB32")

TurnsTile(clip, interlaced=true)