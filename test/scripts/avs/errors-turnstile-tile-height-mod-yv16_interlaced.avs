Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV16")

TurnsTile(clip, tileh=5, interlaced=true)