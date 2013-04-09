//
//          TurnsTile 0.3.2 for AviSynth 2.5.x
//
//  Provides customizable mosaic and palette effects. Latest release
//  hosted at http://www.gyroshot.com/turnstile.htm
//
//          Copyright 2010, 2011, 2013 Robert Martens  robert.martens@gmail.com
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

  PClip clip = args[0][0].AsClip(),
        tilesheet = args[0][1].AsClip();
  VideoInfo vi = clip->GetVideoInfo(),
            vi2 = vi;
  if (tilesheet)
    vi2 = tilesheet->GetVideoInfo();


  int dTileW = 16,
      dTileH = 16;

  int clipW = vi.width,
      clipH = vi.height,
      sheetW = 0,
      sheetH = 0;
  if (tilesheet)
    sheetW = vi2.width, sheetH = vi2.height;

  int lumaW, lumaH;

  if (vi.IsYUV())
    lumaW = 2;
  else
    lumaW = 1;

  if (vi.IsYV12())
    lumaH = 2;
  else
    lumaH = 1;

  // Reduce each default tile dimension to the greatest size, less than or equal
  // to its starting value, that's both a factor of the corresponding clip and
  // tilesheet dimensions, and a multiple of the appropriate macropixel axis.
  for (dTileW; clipW % dTileW > 0 || sheetW % dTileW > 0; dTileW -= lumaW) {
  }
  for (dTileH; clipH % dTileH > 0 || sheetH % dTileH > 0; dTileH -= lumaH) {
  }

  // Try to get square tiles, if possible.
  if (dTileW != dTileH && clipW % dTileH == 0 && sheetW % dTileH == 0)
    dTileW = dTileH;
  if (dTileH != dTileW && clipH % dTileW == 0 && sheetH % dTileW == 0)
    dTileH = dTileW;

  int tileW = args[1].AsInt(dTileW),
      tileH = args[2].AsInt(dTileH);

  
  // I've saved most of my error handling for later, but I have to check for a
  // possible divide by zero when calculating tileIdxMax below. Unfortunately,
  // I also need to add another pair of checks, and the associated prep work,
  // to prevent showing users a tile size warning before establishing that the
  // clip they're using is even valid. Please excuse the out of place code.
  
  // Reading arguments out of order makes me feel icky, but I need this early.
  bool interlaced = args[8].AsBool(false);  
  
  const char* const interlacedStr = interlaced ? "interlaced " : "";
  const char* const cspStr =  vi.IsRGB() ?    "RGB" :
                              vi.IsYUY2() ?   "YUY2" :
                              vi.IsYV12() ?   "YV12" :
                              interlaced ?    "" :
                                              "this";  
  
  int minTileW = lumaW,
      minTileH = lumaH;
  if (interlaced)
    minTileH *= 2;

  if (interlaced) {

    if (clipH % minTileH > 0)
      env->ThrowError(
        "TurnsTile: %s clip height must be mod %d when interlaced=true!",
        cspStr, minTileH);

    if (sheetH % minTileH > 0)
      env->ThrowError(
        "TurnsTile: %s tilesheet height must be mod %d when interlaced=true!",
        cspStr, minTileH);

  }

  if (tileW < minTileW)
    env->ThrowError(
      "TurnsTile: tilew must be at least %d for %s input!",
      minTileW, cspStr);

  if (tileH < minTileH)
    env->ThrowError(
      "TurnsTile: tileh must be at least %d for %s%s input!",
      minTileH, interlacedStr, cspStr);


  int res = args[3].AsInt(8);


  int mode = args[4].AsInt(0);


  const char* levels = env->Invoke("LCase",args[5].AsString("pc")).AsString();


  int loTile = args[6].AsInt(0);

  // Doesn't make a difference here whether interlaced is true or not, since if
  // that's the case we'll later be halving both the tilesheet height and tile
  // height. A / B == (A/2) / (B/2), so tileIdxMax is the same either way.
  int tileIdxMax;
  if (tilesheet)
    tileIdxMax = ((sheetW / tileW) * (sheetH / tileH)) - 1;
  else
    tileIdxMax = 255;

  int hiTile = args[7].AsInt(tileIdxMax);


  PClip finalClip;

  if (tilesheet) {

    if ( !vi.IsSameColorspace(vi2) )
      env->ThrowError("TurnsTile: c and tilesheet must share a colorspace!");

    if (vi.IsRGB32() && (mode < 0 || mode > 4))
      env->ThrowError("TurnsTile: RGB32 only allows modes 0-4!");

    else if (vi.IsRGB24() && (mode < 0 || mode > 3))
      env->ThrowError("TurnsTile: RGB24 only allows modes 0-3!");

    else if (vi.IsYUY2() && (mode < 0 || mode > 4))
      env->ThrowError("TurnsTile: YUY2 only allows modes 0-4!");

    else if (vi.IsYV12() && (mode < 0 || mode > 6))
      env->ThrowError("TurnsTile: YV12 only allows modes 0-6!");    

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

    if (loTile > hiTile)
      env->ThrowError("TurnsTile: lotile cannot be greater than hitile!");

    if (interlaced)
      tileH /= 2;

    finalClip = new TurnsTile(  clip,
                                tilesheet,
                                tileW,
                                tileH,
                                res,
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
      env->ThrowError(
        "TurnsTile: For %s%s input, tileh must be a multiple of %d!",
        interlacedStr, cspStr, minTileH);

    if (clipW % tileW > 0)
      env->ThrowError("TurnsTile: For this clip, tilew must be a factor of %d!",
                      clipW);
    
    if (clipH % tileH > 0)
      env->ThrowError("TurnsTile: For this clip, tileh must be a factor of %d!",
                      clipH);

    if (interlaced)
      tileH /= 2;

    finalClip = new TurnsTile(  clip,
                                tileW,
                                tileH,
                                res,
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

  env->AddFunction("TurnsTile", "c+[tileW]i[tileH]i[res]i[mode]i[levels]s"
                                "[lotile]i[hitile]i[interlaced]b",
                                Create_TurnsTile, 0);

  return "`TurnsTile' - Mosaic and palette effects";

}