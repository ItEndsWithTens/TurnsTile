Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="Y8")

TurnsTile(clip, tileh=0, interlaced=true)