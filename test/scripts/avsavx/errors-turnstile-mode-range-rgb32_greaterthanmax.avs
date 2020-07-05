Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="RGB32")

TurnsTile(clip, mode=100)