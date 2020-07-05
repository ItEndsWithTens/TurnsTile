Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="YUY2")

TurnsTile(clip, interlaced=true)