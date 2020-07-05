Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=478, pixel_type="YV12")

TurnsTile(clip, interlaced=true)