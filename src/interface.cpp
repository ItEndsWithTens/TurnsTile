//
//          TurnsTile 0.3.0 for AviSynth 2.5.x
//
//  Provides customizable mosaic and palette effects. Latest release
//  hosted at http://www.gyroshot.com/turnstile.htm
//
//          Copyright 2010-2011 Robert Martens  robert.martens@gmail.com
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.



#include "CLUTer.h"
#include "TurnsTile.h"

#include <cmath>
#include <Windows.h>

#include "avisynth.h"



AVSValue __cdecl Create_TurnsTile(AVSValue args, void* user_data, IScriptEnvironment* env)
{

  VideoInfo viCreate = args[0].AsClip()->GetVideoInfo();

  if (viCreate.IsRGB() == false &&
      viCreate.IsYUY2() == false &&
      viCreate.IsYV12() == false)
    env->ThrowError("TurnsTile: Only RGB, YUY2, and YV12 input supported!");

  char* cspStrW = viCreate.IsRGB() ?  "RGB" :
                  viCreate.IsYUY2() ? "YUY2" :
                  viCreate.IsYV12() ? "YV12" :
                                      "this";
  
  const int DEFAULT_TILESIZE = 16;

  int minTileW =  viCreate.IsYUY2() || viCreate.IsYV12() ? 2 : 1;

  PClip finalClip;

  if (args[1].IsClip()) { // With user provided tilesheet

    VideoInfo vi2Create = args[1].AsClip()->GetVideoInfo();

    bool interlaced = args[9].AsBool(false);

    // I handle the first four conditions here myself, as if left to AviSynth,
    // the error won't show until I invoke SeparateFields() in my constructors'
    // initializer lists. As a user, I'd be confused if I got a SeparateFields
    // error when I hadn't explicitly called that function, hence the checks.
    if (interlaced && viCreate.IsYV12() && viCreate.height % 4 != 0)
      env->ThrowError("TurnsTile: YV12 clip height must be mod 4 "
                      "when interlaced=true!");

    if (interlaced && vi2Create.IsYV12() && vi2Create.height % 4 != 0)
      env->ThrowError("TurnsTile: YV12 tilesheet height must be mod 4 "
                      "when interlaced=true!");

    if (interlaced && viCreate.height % 2 != 0)
      env->ThrowError("TurnsTile: Clip height must be even "
                      "when interlaced=true!");

    if (interlaced && vi2Create.height % 2 != 0)
      env->ThrowError("TurnsTile: Tilesheet height must be even "
                      "when interlaced=true!");

    ////

    if ( !viCreate.IsSameColorspace(vi2Create) )
      env->ThrowError("TurnsTile: c and tilesheet must share a colorspace!");

    int mode = args[5].AsInt(0);

    if (viCreate.IsRGB32() && (mode < 0 || mode > 4))
      env->ThrowError("TurnsTile: RGB32 only allows modes 0-4!");

    else if (viCreate.IsRGB24() && (mode < 0 || mode > 3))
      env->ThrowError("TurnsTile: RGB24 only allows modes 0-3!");

    else if (viCreate.IsYUY2() && (mode < 0 || mode > 4))
      env->ThrowError("TurnsTile: YUY2 only allows modes 0-4!");

    else if (viCreate.IsYV12() && (mode < 0 || mode > 6))
      env->ThrowError("TurnsTile: YV12 only allows modes 0-6!");
    
    // I'm supremely disappointed with the complexity of this section, but the
    // addition to version 0.3.0 of both the 'interlaced' parameter, which needs
    // mod 2 height, and YV12, with its vertical chroma subsampling demanding
    // mod 2 progressive height and mod 4 interlaced, made the auto calculation
    // of tile size much more complicated. There's most likely a smarter way to
    // do this, but I'm burned out for the moment.
    int clipW =   viCreate.width,
        sheetW =  vi2Create.width,
        clipH =   viCreate.height,
        sheetH =  vi2Create.height;

    int minTileH =  viCreate.IsYV12() && interlaced ? 4 :
                    viCreate.IsYV12() || interlaced ? 2 :
                    1;

    // Just in case you decide to change DEFAULT_TILESIZE, I try to ensure it's
    // still a multiple of the minimum for the colorspace in question.
    int tileW = (DEFAULT_TILESIZE / minTileW) * minTileW,
        tileH = (DEFAULT_TILESIZE / minTileH) * minTileH;

    for (tileW; clipW % tileW > 0 || sheetW % tileW > 0; tileW -= minTileW) {
    }

    for (tileH; clipH % tileH > 0 || sheetH % tileH > 0; tileH -= minTileH) {
    }

    // After looping through the tile width and height, each is set to the
    // highest value below 16 that's still a multiple of the appropriate clip
    // dimension. Now, through a pair of ugly, brute force checks, I try to
    // get square tiles if possible. I remind you I'm unhappy with this.
    if (tileW != tileH && clipW % tileH == 0 && sheetW % tileH == 0)
      tileW = tileH;

    if (tileH != tileW && clipH % tileW == 0 && sheetH % tileW == 0)
      tileH = tileW;

    tileW = args[2].AsInt(tileW <= minTileW ? minTileW : tileW);
    tileH = args[3].AsInt(tileH <= minTileH ? minTileH : tileH);

    ////

    if (tileW < minTileW)
      env->ThrowError("TurnsTile: tilew must be at least %d for %s input!",
                      minTileW, cspStrW);

    char* cspStrH = viCreate.IsYV12() && interlaced ? "interlaced YV12" :
                    interlaced ?                      "interlaced" :
                    viCreate.IsRGB() ?                "RGB" :
                    viCreate.IsYUY2() ?               "YUY2" :
                    viCreate.IsYV12() ?               "YV12" :
                                                      "this";

    if (tileH < minTileH)
      env->ThrowError("TurnsTile: tileh must be at least "
                      "%d for %s input!", minTileH, cspStrH);

    int gcfW = TurnsTile_gcf(clipW, sheetW),
        gcfH = TurnsTile_gcf(clipH, sheetH);

    if (tileW > gcfW)
      env->ThrowError("TurnsTile: For this clip and tilesheet, "
                      "tilew must not exceed %d!", gcfW);
    
    if (tileH > gcfH)
      env->ThrowError("TurnsTile: For this clip and tilesheet, "
                      "tileh must not exceed %d!", gcfH);

    if (tileW % minTileW > 0)
      env->ThrowError("TurnsTile: For this clip, tilew must be a multiple of %d!",
                      minTileW);
    
    if (tileH % minTileH > 0)
      env->ThrowError("TurnsTile: For this clip, tileh must be a multiple of %d!",
                      minTileH);

    if (gcfW % tileW > 0)
      env->ThrowError("TurnsTile: For this clip and tilesheet, "
                      "tilew must be a factor of %d!", gcfW);

    if (gcfH % tileH > 0)
      env->ThrowError("TurnsTile: For this clip and tilesheet, "
                      "tileh must be a factor of %d!", gcfH);

    const char* levels = env->Invoke("LCase",args[6].AsString("pc")).AsString();

    int sheetCols = vi2Create.width / tileW,
        sheetRows = vi2Create.height / tileH,
        sheetTiles = sheetCols * sheetRows,
        tileIdxMax = sheetTiles - 1;

    int loTile =  args[7].AsInt(0) <= 0 ? 0 :
                  args[7].AsInt() >= tileIdxMax ? tileIdxMax :
                  args[7].AsInt();

    int hiTile =  args[8].AsInt(tileIdxMax) >= tileIdxMax ? tileIdxMax :
                  args[8].AsInt() <= 0 ? 0 :
                  args[8].AsInt();

    if (loTile > hiTile)
      env->ThrowError("TurnsTile: lotile cannot be greater than hitile!");

    if (interlaced)
      tileH /= 2;

    finalClip = new TurnsTile(  args[0].AsClip(),       // c
                                args[1].AsClip(),       // tilesheet
                                tileW,
                                tileH,
                                args[4].AsInt(8),       // res
                                mode,
                                levels,
                                loTile,
                                hiTile,
                                interlaced,
                                env);

    return  interlaced && finalClip->GetVideoInfo().IsFieldBased() ?
              env->Invoke("Weave", finalClip) :
            finalClip;

  } else { // No tilesheet

    bool interlaced = args[8].AsBool(false);

    if (viCreate.IsYV12() && interlaced && viCreate.height % 4 != 0)
      env->ThrowError("TurnsTile: YV12 height must be mod 4 when interlaced=true!");

    if (interlaced && viCreate.height % 2 != 0)
      env->ThrowError("TurnsTile: Height must be even when interlaced=true!");

    ////

    int clipW = viCreate.width,
        clipH = viCreate.height;

    int minTileH =  viCreate.IsYV12() && interlaced ? 4 :
                    viCreate.IsYV12() || interlaced ? 2 :
                    1;

    int tileW = (DEFAULT_TILESIZE / minTileW) * minTileW,
        tileH = (DEFAULT_TILESIZE / minTileH) * minTileH;

    for (tileW; clipW % tileW > 0; tileW -= minTileW) {
    }

    for (tileH; clipH % tileH > 0; tileH -= minTileH) {
    }

    if (tileW != tileH && clipW % tileH == 0 && tileH % minTileW == 0)
      tileW = tileH;

    if (tileH != tileW && clipH % tileW == 0 && tileW % minTileH == 0)
      tileH = tileW;

    tileW = args[1].AsInt(tileW <= minTileW ? minTileW : tileW);
    tileH = args[2].AsInt(tileH <= minTileH ? minTileH : tileH);

    ////

    if (tileW < minTileW)
      env->ThrowError("TurnsTile: tilew must be at least %d for %s input!",
                      minTileW, cspStrW);

    char* cspStrH = viCreate.IsYV12() && interlaced ? "interlaced YV12" :
                    interlaced ?                      "interlaced" :
                    viCreate.IsRGB() ?                "RGB" :
                    viCreate.IsYUY2() ?               "YUY2" :
                    viCreate.IsYV12() ?               "YV12" :
                                                      "this";

    if (tileH < minTileH)
      env->ThrowError("TurnsTile: tileh must be at least %d for %s input!",
                      minTileH, cspStrH);

    if (tileW > clipW)
      env->ThrowError("TurnsTile: For this clip, tilew must not exceed %d!",
                      clipW);
    
    if (tileH > clipH)
      env->ThrowError("TurnsTile: For this clip, tileh must not exceed %d!",
                      clipH);
        
    if (tileW % minTileW > 0)
      env->ThrowError("TurnsTile: For this clip, tilew must be a multiple of %d!",
                      minTileW);
    
    if (tileH % minTileH > 0)
      env->ThrowError("TurnsTile: For this clip, tileh must be a multiple of %d!",
                      minTileH);

    if (clipW % tileW > 0)
      env->ThrowError("TurnsTile: For this clip, tilew must be a factor of %d!",
                      clipW);
    
    if (clipH % tileH > 0)
      env->ThrowError("TurnsTile: For this clip, tileh must be a factor of %d!",
                      clipH);

    int loTile =  args[6].AsInt(0) <= 0 ? 0 :
                  args[6].AsInt() >= 255 ? 255 :
                  args[6].AsInt();

    int hiTile =  args[7].AsInt(255) >= 255 ? 255:
                  args[7].AsInt() <= 0 ? 0 :
                  args[7].AsInt();

    if (interlaced)
      tileH /= 2;

    finalClip = new TurnsTile(  args[0].AsClip(), // c
                                tileW,
                                tileH,
                                args[3].AsInt(8), // res
                                loTile,
                                hiTile,
                                interlaced,
                                env);

    return  interlaced && finalClip->GetVideoInfo().IsFieldBased() ?
              env->Invoke("Weave", finalClip) :
            finalClip;

  }

}



AVSValue __cdecl Create_CLUTer(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  
  VideoInfo viCreate = args[0].AsClip()->GetVideoInfo();

  if (viCreate.IsRGB() == false &&
      viCreate.IsYUY2() == false &&
      viCreate.IsYV12() == false)
    env->ThrowError("CLUTer: Only RGB, YUY2, and YV12 input supported!");

  bool interlaced = args[3].AsBool(false);

  if ( !viCreate.IsSameColorspace(args[1].AsClip()->GetVideoInfo()) )
    env->ThrowError("CLUTer: c and palette must share a colorspace!");

  if (viCreate.IsYV12() && interlaced && viCreate.height % 4 != 0)
    env->ThrowError("CLUTer: YV12 height must be mod 4 when interlaced=true!");

  if (interlaced && viCreate.height % 2 != 0)
    env->ThrowError("CLUTer: Height must be even when interlaced=true!");  

  PClip finalClip = new CLUTer(  args[0].AsClip(), // c
                                 args[1].AsClip(), // palette
                                 args[2].AsInt(0), // paletteframe
                                 interlaced,
                                 env);

  return  interlaced && finalClip->GetVideoInfo().IsFieldBased() ?
            env->Invoke("Weave", finalClip) :
          finalClip;

}



extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{

  env->AddFunction("CLUTer", "cc[paletteframe]i[interlaced]b",
                                  Create_CLUTer, 0);

  env->AddFunction("TurnsTile", "cc[tileW]i[tileH]i[res]i[mode]i[levels]s"
                                "[lotile]i[hitile]i[interlaced]b",
                                Create_TurnsTile, 0);

  // Although this version of TurnsTile, without a user-supplied tilesheet,
  // doesn't end up using mode or levels, I still include them, in an effort to
  // prevent "TurnsTile doesn't have a named argument 'Foo'" errors. Instead,
  // they're just dummies, and Create_TurnsTile up above simply ignores them if
  // calling the constructor with no sheet.
  env->AddFunction("TurnsTile", "c[tileW]i[tileH]i[res]i[mode]i[levels]s"
                                "[lotile]i[hitile]i[interlaced]b",
                                Create_TurnsTile, 0);
  
  return "`TurnsTile' - Mosaic and palette effects";

}