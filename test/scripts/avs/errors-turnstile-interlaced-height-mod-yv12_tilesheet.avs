Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV12")
tilesheet = BlankClip(height=478, pixel_type="YV12")

TurnsTile(clip, tilesheet, interlaced=true)