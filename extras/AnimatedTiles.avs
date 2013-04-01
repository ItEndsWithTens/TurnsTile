pixel_type="RGB32"
fps = 30000.0/1001.0

white =		ImageSource("gray_16x16.png",end=30,pixel_type=pixel_type,fps=fps)
red =		ImageSource("red_16x16.png",end=30,pixel_type=pixel_type,fps=fps)
orange =	Overlay(ImageSource("red_16x16.png",end=30,pixel_type=pixel_type,fps=fps),ImageSource("yellow_16x16.png",end=30,pixel_type=pixel_type,fps=fps),opacity=.5)
yellow =	ImageSource("yellow_16x16.png",end=30,pixel_type=pixel_type,fps=fps)
green =		ImageSource("green_16x16.png",end=30,pixel_type=pixel_type,fps=fps)
blue =		ImageSource("blue_16x16.png",end=30,pixel_type=pixel_type,fps=fps)
indigo =	Overlay(ImageSource("blue_16x16.png",end=30,pixel_type=pixel_type,fps=fps),ImageSource("magenta_16x16.png",pixel_type=pixel_type,fps=fps),opacity=.5)
violet =	ImageSource("magenta_16x16.png",end=30,pixel_type=pixel_type,fps=fps)

Dissolve(red,orange,yellow,green,blue,indigo,violet,red,15)
Loop(start=15,end=125)