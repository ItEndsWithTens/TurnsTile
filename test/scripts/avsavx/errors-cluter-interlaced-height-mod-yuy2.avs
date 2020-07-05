Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="YUY2")
palette = BlankClip(height=479, pixel_type="YUY2")

CLUTer(clip, palette, interlaced=true)