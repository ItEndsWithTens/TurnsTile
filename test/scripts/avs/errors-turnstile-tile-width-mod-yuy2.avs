Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YUY2")

TurnsTile(clip, tilew=5)