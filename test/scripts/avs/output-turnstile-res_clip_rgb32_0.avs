Import("util.avs")
InitializeTurnsTileTestEnvironment()

TurnsTileTestSource("../clips/hsl.png")
ConvertToRGB32()

TurnsTile(1, 1, res=0)