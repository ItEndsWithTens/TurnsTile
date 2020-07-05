Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="Y8")

TurnsTile(clip, interlaced=true)