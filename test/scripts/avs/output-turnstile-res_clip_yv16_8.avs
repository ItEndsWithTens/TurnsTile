Import("util.avs")
InitializeTurnsTileTestEnvironment()

TurnsTileTestSource("../clips/hsl.png")
ConvertToYV16(matrix="PC.601")

TurnsTile(2, 1, res=8)