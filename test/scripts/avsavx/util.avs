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



function MakeDotUnique(string pixel_type)
{
  
  minw = GetMinWidth(pixel_type)
  minh = GetMinHeight(pixel_type)
  
  # I need to generate each pixel as a block, stack them, then PointResize the
  # whole thing down later. The alternative for YUV formats is to generate a
  # bunch of 1x1 pixels in RGB, stack them, then convert to the final colorspace,
  # but for tiny clips you'd get "Source image too small for this resize method".
  base = BlankClip(width=minw, height=minh, pixel_type=pixel_type)
  y1 = base.IsYUV() ? BlankClip(base, color_yuv=$FF40C0) : Undefined()
  y2 = base.IsYUV() ? BlankClip(base, color_yuv=$AA40C0) : Undefined()
  y3 = base.IsYUV() ? BlankClip(base, color_yuv=$5540C0) : Undefined()
  y4 = base.IsYUV() ? BlankClip(base, color_yuv=$0040C0) : Undefined()
  rgb = base.IsRGB32() ? BlankClip(base, color=$40FFAA55) : \
        base.IsRGB24() ? BlankClip(base, color=$FFAA55) : Undefined()
  
  macropixel = FindStr(pixel_type, "YV411") > 0 ? \
                 StackHorizontal(y1, y2, y3, y4) : \
               FindStr(pixel_type, "YV12") > 0 ? \
                 StackVertical(StackHorizontal(y1, y2), \
                               StackHorizontal(y3, y4)) : \
               FindStr(pixel_type, "YV16") > 0 || \
               FindStr(pixel_type, "YUY2") > 0 ?  \
                 StackHorizontal(y2, y3) : \
               FindStr(pixel_type, "YV24") > 0 || \
               FindStr(pixel_type, "Y8") > 0 ? \
                 y2 : \
               rgb

  return macropixel.PointResize(minw, minh)

}



function MakeTileWithDot(int width, int height, val color_dot, int color_bg, \
                         string pixel_type)
{

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

  color_rgb_dot = FindStr(pixel_type, "RGB") > 0 ? color_dot : Undefined()
  color_yuv_dot = FindStr(pixel_type, "RGB") > 0 ? Undefined() : color_dot
  color_rgb_bg = FindStr(pixel_type, "RGB") > 0 ? color_bg : Undefined()
  color_yuv_bg = FindStr(pixel_type, "RGB") > 0 ? Undefined() : color_bg

  top = toph < minh ? \
          NOP() : \
        BlankClip(width=topw, height=toph, color=color_rgb_bg, \
                  color_yuv=color_yuv_bg, pixel_type=pixel_type)

  ctr_l = ctrw_l < minw ? \
            NOP() : \
          BlankClip(width=ctrw_l, height=ctrh_l, color=color_rgb_bg, \
                    color_yuv=color_yuv_bg, pixel_type=pixel_type)

  ctr_dot = color_dot.IsInt() ? \
              BlankClip(width=minw, height=minh, color=color_rgb_dot, \
                        color_yuv=color_yuv_dot, pixel_type=pixel_type) : \
            MakeDotUnique(pixel_type)

  ctr_r = ctrw_r < minw ? \
            NOP() : \
          BlankClip(width=ctrw_r, height=ctrh_r, color=color_rgb_bg, \
                    color_yuv=color_yuv_bg, pixel_type=pixel_type)

  ctr = width == minw ? \
          ctr_dot : \
        Eval("StackHorizontal(" + (ctrw_l > 0 ? "ctr_l," : "") + \
                                  "ctr_dot" + \
                                  (ctrw_r > 0 ? ",ctr_r" : "") + ")")

  bot = both < minh ? \
          NOP() : \
        BlankClip(width=botw, height=both, color=color_rgb_bg, \
                  color_yuv=color_yuv_bg, pixel_type=pixel_type)

  return toph < minh && both < minh ? \
           ctr : \
         Eval("StackVertical(" + (toph > 0 ? "top," : "") + \
                                 "ctr" + \
                                 (both > 0 ? ",bot" : "") + ")")

}



function MakeClip(int width, int height, int color_bg, val color_dot, \
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

  # Calls to BlankClip can have both color and color_yuv set, as long as the one
  # opposite the requested pixel_type is undefined. Using this behavior, I'm able
  # to generate my test clips directly in their target colorspaces, even from a
  # generic function like this one that might be called from RGB or YUV scripts.
  color_rgb_dot = FindStr(pixel_type, "RGB") > 0 ? color_dot : Undefined()
  color_yuv_dot = FindStr(pixel_type, "RGB") > 0 ? Undefined() : color_dot
  color_rgb_bg = FindStr(pixel_type, "RGB") > 0 ? color_bg : Undefined()
  color_yuv_bg = FindStr(pixel_type, "RGB") > 0 ? Undefined() : color_bg

  fill = BlankClip(width=width-(tilew*2), height=tileh, color=color_rgb_bg, \
                   color_yuv=color_yuv_bg, pixel_type=pixel_type)

  edge = Eval("StackHorizontal(" + (dot.Width() > 0 ? "dot" : "") + \
                                   (fill.Width() > 0 ? ", fill" : "") + \
                                   (dot.Width() > 0 ? ", dot" : "") + ")")

  ctr = BlankClip(width=width, height=height-(tileh*2), color=color_rgb_bg, \
                  color_yuv=color_yuv_bg, pixel_type=pixel_type)

  return Eval("StackVertical(" + (edge.Height() > 0 ? "edge" : "") + \
                                 (ctr.Height() > 0 ? ", ctr" : "") + \
                                 (edge.Height() > 0 ? ", edge" : "") + ")")

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