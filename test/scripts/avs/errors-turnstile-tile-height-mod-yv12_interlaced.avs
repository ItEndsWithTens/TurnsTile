Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV12")

TurnsTile(clip, tileh=5, interlaced=true)