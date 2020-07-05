Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="YV24")
palette = BlankClip(height=479, pixel_type="YV24")

CLUTer(clip, palette, interlaced=true)