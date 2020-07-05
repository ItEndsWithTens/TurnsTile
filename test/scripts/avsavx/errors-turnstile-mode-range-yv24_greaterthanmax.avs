Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV24")

TurnsTile(clip, mode=100)