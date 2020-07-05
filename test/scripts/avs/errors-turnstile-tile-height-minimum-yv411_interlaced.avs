Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV411")

TurnsTile(clip, tileh=0, interlaced=true)