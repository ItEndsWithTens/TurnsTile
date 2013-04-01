pixel_type="RGB32"

white =		ImageSource("gray_16x16.png",pixel_type=pixel_type)
red =		ImageSource("red_16x16.png",pixel_type=pixel_type)
orange =	Overlay(ImageSource("red_16x16.png",pixel_type=pixel_type),ImageSource("yellow_16x16.png",pixel_type=pixel_type),opacity=.5)
yellow =	ImageSource("yellow_16x16.png",pixel_type=pixel_type)
green =		ImageSource("green_16x16.png",pixel_type=pixel_type)
blue =		ImageSource("blue_16x16.png",pixel_type=pixel_type)
indigo =	Overlay(ImageSource("blue_16x16.png",pixel_type=pixel_type),ImageSource("magenta_16x16.png",pixel_type=pixel_type),opacity=.5)
violet =	ImageSource("magenta_16x16.png",pixel_type=pixel_type)

StackVertical(	violet.Crop(0,0,256,32),
\		indigo.Crop(0,32,256,32),
\		blue.Crop(0,64,256,32),
\		green.Crop(0,96,256,32),
\		yellow.Crop(0,128,256,32),
\		orange.Crop(0,160,256,32),
\		red.Crop(0,192,256,32),
\		white.Crop(0,224,256,32))