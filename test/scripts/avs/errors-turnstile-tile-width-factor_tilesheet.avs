Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(width=640)
tilesheet = BlankClip(width=256)

TurnsTile(clip, tilesheet, tilew=17) #Should be 19?