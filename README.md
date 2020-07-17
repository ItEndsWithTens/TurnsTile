  TurnsTile
  =========

  A mosaic and palette effects plugin for [Avisynth](http://www.avisynth.nl).

  [![Build Status](https://dev.azure.com/robertmartens0491/TurnsTile/_apis/build/status/ItEndsWithTens.TurnsTile?repoName=ItEndsWithTens%2FTurnsTile&branchName=master)](https://dev.azure.com/robertmartens0491/TurnsTile/_build/latest?definitionId=4&repoName=ItEndsWithTens%2FTurnsTile&branchName=master)

  ### Requirements ###
  1. Avisynth 2.6 or greater, or Avisynth+
  2. [**Windows**] Microsoft's Visual C++ 2019 Redistributable - [x64](https://aka.ms/vs/16/release/VC_redist.x64.exe) - [x86](https://aka.ms/vs/16/release/VC_redist.x86.exe)

  http://www.gyroshot.com/turnstile.htm  
  robert.martens@gmail.com  
  [@ItEndsWithTens](https://twitter.com/ItEndsWithTens)

  ### TurnsTile ###

    TurnsTile(clip c, clip "tilesheet", int "tilew", int "tileh", int "res",
              int "mode", string "levels", int "lotile", int "hitile",
              bool "interlaced")

  **c** clip
  - The input clip, which can be RGB32, RGB24, YUY2, or YV12.

  **tilesheet** clip
  - Optional; if supplied, tiles will be pulled from this clip, which must be in  
    the same colorspace as 'c'. Your tilesheet can be a still image or a video.  
    In the latter case, tiles for a given frame of 'c' will be clipped from the  
    corresponding frame of 'tilesheet'.

    The tiles are numbered left to right, then top to bottom. Using the provided  
    tilesheets as examples, with 16x16 pixel tiles in a 256x256 pixel image, the  
    top left tile is number 0, top right is 15, bottom left is 240, and bottom  
    right is 255. Design your own custom images accordingly, with dark tones at  
    the top left fading up to lighter ones at the bottom right (first left to  
    right across a row, then top to bottom one row at a time).

  **tilew, tileh** int, default largest size <= 16x16 that fits your input
  - If your tiles aren't sixteen by sixteen, define custom values here. Each  
    must be a factor of the respective clip dimension.

  **res** int, default 8
  - This acts as the effective bit depth of your output. The range of possible  
    output values is broken up into 2 ** res steps, and each tile index or pixel  
    component is rounded accordingly. This is quite an effective technique for  
    RGB footage, but thanks to the way color is represented in YUV spaces, you  
    won't be able to lower this too much with YUY2 or YV12 input before things  
    begin to look strange.

  **mode** integer, default 0
  - Only works when tilesheet is supplied. This option chooses the component  
    that will serve as the tile index for a given tile. The possible values are  
    as follows:

        0:
          RGB:   Average of red, green, and blue values
          YUY2:  Average of Y1 and Y2 in the current pixel pair
          YV12:  Average of all four Y values in the 2x2 block

        1:
          RGB:   Blue
          YUY2:  Y1
          YV12:  Top left

        2:
          RGB:   Green
          YUY2:  U
          YV12:  Top right

        3:
          RGB:   Red
          YUY2:  Y2
          YV12:  Bottom left

        4:
          RGB32: Alpha
          RGB24: N/A
          YUY2:  V
          YV12:  Bottom right

        5:
          RGB32: N/A
          RGB24: N/A
          YUY2:  N/A
          YV12:  U

        6:
          RGB32: N/A
          RGB24: N/A
          YUY2:  N/A
          YV12:  V

  **levels** string, "pc" or "tv", default "pc"
  - Which range to use when selecting tiles. If you'd like to map TV black and  
    white to the lowest and highest tiles in your tilesheet, respectively, use  
    "tv" instead of the default "pc".

  **lotile, hitile** default 0 and tilecount - 1
  - A quick way to limit the tile selection to a given portion of your  
    tilesheet; if for example you have a sheet with an odd number of tiles, and  
    some spaces are blank, or if you just want to use a smaller range of values  
    without having to rebuild your tilesheet by hand, use these. If you don't  
    use a tilesheet, these will instead control the maximum and minimum  
    component values.

  **interlaced** bool, default false
  - Enable for interlaced input. For those unaware, "interlaced" and "field  
    based" are not the same thing. If a clip is field based, it's more than  
    likely interlaced, but the reverse isn't true, and there's currently no  
    completely fool proof way to auto-detect interlaced input.

  ----

  ### CLUTer ###
    CLUTer(clip c, clip palette, int "paletteframe", bool "interlaced")

  **c** clip
  - No special restrictions, beyond ensuring that this clip's colorspace  
    matches that of your palette. RGB32, RGB24, YUY2, and YV12 are supported.

  **palette** clip
  - Must be the same colorspace as the input clip. Progressive chroma siting is  
    assumed for YV12 palettes; although 'c' will be treated as interlaced if the  
    appropriate option is enabled, the palette itself will always be read as if  
    progressive.

    **CAUTION!** It is very, very easy to dramatically slow down the operation  
    of CLUTer, to such a degree that it may appear to be frozen. The short  
    version: stick with ~100 or fewer unique colors per palette, or you're in  
    for a long wait. If you want to use an image as your palette, run it through  
    TurnsTile first, with big tiles and/or a lowered 'res'.

  **paletteframe** int, default 0
  - Only one frame is used from any clip you pass in as your palette, so if you  
    don't want to use the colors of frame 0, set paletteframe accordingly.

  **interlaced** bool, default false
  - As explained above, the terms "field based" and "interlaced" are not  
    necessarily synonymous. Reading the proper luma values for a given chroma  
    sample requires knowing the nature of the input clip, and a user-defined  
    parameter is the most reliable way to achieve that.

  ----

  ### Extras ###

  Included in the 'extras' directory is a set of tilesheets meant to serve as a  
  jumping off point for your own experimentation, along with the AviSynth script  
  used to generate them. It needs [Gavino's GScript](http://forum.doom9.org/showthread.php?t=147846) to make it work in Avisynth  
  2.6, but there are no other requirements.

  CGApalette.avs was introduced along with CLUTer() in version 0.3.0, and as  
  stated in the comments is meant to be loaded by AVISource as a sample palette  
  for that function. Open up the script to find more details.

  Lastly, also in the extras folder is a copy of the script version of TurnsTile  
  0.1.0. Use in Aviysnth 2.6 requires GScript, along with [GRunT](http://forum.doom9.org/showthread.php?t=139337), also by Gavino,  
  and doesn't run very quickly, to say the least. That barely measurable speed  
  was the motivation to develop this plugin, and the script is only included  
  here for educational purposes.
