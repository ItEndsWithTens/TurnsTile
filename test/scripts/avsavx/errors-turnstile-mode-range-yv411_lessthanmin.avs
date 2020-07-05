Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV411")

TurnsTile(clip, mode=-1)