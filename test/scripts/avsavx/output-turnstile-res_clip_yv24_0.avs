Import("util.avs")

TurnsTileTestSource("../clips/hsl.png")
ConvertToYV24(matrix="PC.601")

TurnsTile(1, 1, res=0)