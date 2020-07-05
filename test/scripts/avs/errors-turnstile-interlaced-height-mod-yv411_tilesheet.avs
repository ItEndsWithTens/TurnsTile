Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(pixel_type="YV411")
tilesheet = BlankClip(height=479, pixel_type="YV411")

TurnsTile(clip, tilesheet, interlaced=true)