Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="RGB24")

TurnsTile(clip, mode=100)