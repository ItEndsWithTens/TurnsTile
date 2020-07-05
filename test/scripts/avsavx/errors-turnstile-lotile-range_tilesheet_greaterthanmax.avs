Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip()
tilesheet = BlankClip(width=1024, height=1024)

TurnsTile(clip, tilesheet, lotile=4096)