Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="YV411")

TurnsTile(clip, interlaced=true)