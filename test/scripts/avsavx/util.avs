function Mod(float num, int mod, int dir)
{

  return dir > 0 ? \
           Ceil(num / Float(mod)) * mod : \
         dir < 0 ? \
           Floor(num / Float(mod)) * mod : \
         Round(num / Float(mod)) * mod

}



function GetMinWidth(string pixel_type)
{

  pixel_type = UCase(pixel_type)

  return FindStr(pixel_type, "YV411") > 0 ? \
           4 : \
         FindStr(pixel_type, "YV16") > 0 || \
         FindStr(pixel_type, "YV12") > 0 || \
         FindStr(pixel_type, "YUY2") > 0 ?  \
           2 : \
         1

}



function GetMinHeight(string pixel_type)
{

  pixel_type = UCase(pixel_type)

  return FindStr(pixel_type, "YV12") > 0 ? 2 : 1

}



function MakeTileWithDot(int width, int height, int color_dot, int color_bg, \
                         string pixel_type)
{

  # For reasons explained in a comment in MakeClip, below, I build and return an
  # RGB24 clip here. I pass in pixel_type nonetheless since I need to determine
  # the various dimensions of this tile's pieces, and specifying ten parameters
  # just to call this function is a little unwieldy.

  minw = GetMinWidth(pixel_type)
  minh = GetMinHeight(pixel_type)

  topw = width
  toph = FindStr(pixel_type, "RGB") > 0 ? \
           Mod(height / 2.0, minh, 1) - minh : \
         Mod(height / 2.0, minh, -1)

  ctrw_l = Mod(width / 2.0, minw, -1)
  ctrw_r = Mod(width / 2.0, minw, 1) - minw
  ctrh_l = minh  
  ctrh_r = minh

  botw = width
  both = FindStr(pixel_type, "RGB") > 0 ? \
           Mod(height / 2.0, minh, -1) : \
         Mod(height / 2.0, minh, 1) - minh

  top = toph < minh ? \
          NOP() : \
        BlankClip(width=topw, height=toph, color=color_bg, pixel_type="RGB24")

  ctr_l = ctrw_l < minw ? \
            NOP() : \
          BlankClip(width=ctrw_l, height=ctrh_l, color=color_bg, \
                    pixel_type="RGB24")

  ctr_dot = BlankClip(width=minw, height=minh, color=color_dot, \
                      pixel_type="RGB24")

  ctr_r = ctrw_r < minw ? \
            NOP() : \
          BlankClip(width=ctrw_r, height=ctrh_r, color=color_bg, \
                    pixel_type="RGB24")

  ctr = width == minw ? \
          ctr_dot : \
        Eval("StackHorizontal(" + (ctrw_l > 0 ? "ctr_l," : "") + \
                                  "ctr_dot" + \
                                  (ctrw_r > 0 ? ",ctr_r" : "") + ")")

  bot = both < minh ? \
          NOP() : \
        BlankClip(width=botw, height=both, color=color_bg, pixel_type="RGB24")

  return toph < minh && both < minh ? \
           ctr : \
         Eval("StackVertical(" + (toph > 0 ? "top," : "") + \
                                 "ctr" + \
                                 (both > 0 ? ",bot" : "") + ")")

}



function MakeClip(int width, int height, int color_bg, int color_dot, \
                  int color_dot_bg, string pixel_type)
{

  # This function is only for my use, and I know the relationship between
  # width/height and tilew/tileh (the former being three times the latter). This
  # means I can compute one set of variables from the other, so the choice then
  # becomes which two the user should specify. When I call this function, I
  # already know tilew and tileh, but it seems unintuitive to have a function
  # called MakeClip whose first two parameters are not the dimensions of said
  # clip, so I decided to make tilew and tileh the implicit variables.
  tilew = width / 3
  tileh = height / 3

  dot = MakeTileWithDot(tilew, tileh, color_dot, color_dot_bg, pixel_type)

  fill = BlankClip(width=width-(tilew*2), height=tileh, color=color_bg, \
                   pixel_type="RGB24")

  edge = Eval("StackHorizontal(" + (dot.Width() > 0 ? "dot" : "") + \
                                   (fill.Width() > 0 ? ", fill" : "") + \
                                   (dot.Width() > 0 ? ", dot" : "") + ")")

  ctr = BlankClip(width=width, height=height-(tileh*2), color=color_bg, \
                  pixel_type="RGB24")

  final = Eval("StackVertical(" + (edge.Height() > 0 ? "edge" : "") + \
                                  (ctr.Height() > 0 ? ", ctr" : "") + \
                                  (edge.Height() > 0 ? ", edge" : "") + ")")
                                 
  # I create the test clip in RGB24 and only later convert it to the desired
  # colorspace: when specifying a YUV pixel_type in BlankClip, it starts with
  # RGB24 and converts the clip itself, assuming the Rec601 matrix. I want
  # PC.601 instead, but BlankClip offers no way to choose that directly.
  return Eval("final.ConvertTo" + pixel_type + """(matrix="PC.601")""")

}



function MakeTilesheet(int tilew, int tileh, string pixel_type)
{

  r = BlankClip(width=tilew, height=tileh, color=$FF0000, pixel_type=pixel_type)
  g = BlankClip(width=tilew, height=tileh, color=$00FF00, pixel_type=pixel_type)
  b = BlankClip(width=tilew, height=tileh, color=$0000FF, pixel_type=pixel_type)
  c = BlankClip(width=tilew, height=tileh, color=$00FFFF, pixel_type=pixel_type)
  m = BlankClip(width=tilew, height=tileh, color=$FF00FF, pixel_type=pixel_type)
  y = BlankClip(width=tilew, height=tileh, color=$FFFF00, pixel_type=pixel_type)

  top = StackHorizontal(r, g, b)
  bot = StackHorizontal(c, m, y)

  return StackVertical(top, bot)

}