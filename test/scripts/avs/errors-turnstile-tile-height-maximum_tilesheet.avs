Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=480)
tilesheet = BlankClip(height=256)

TurnsTile(clip, tilesheet, tileh=500)