Import("util.avs")
InitializeTurnsTileTestEnvironment()

TurnsTileTestSource("../clips/hsl.png")
ConvertToY8(matrix="PC.601")

TurnsTile(1, 1, res=8)