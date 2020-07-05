Import("util.avs")
InitializeTurnsTileTestEnvironment()

clip = BlankClip(height=479, pixel_type="Y8")
palette = BlankClip(height=479, pixel_type="Y8")

CLUTer(clip, palette, interlaced=true)