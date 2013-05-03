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
#include <cstring>

#include "interface.h"



#if defined(TURNSTILE_HOST_AVXSYNTH)

using avxsynth::AVSValue;
using avxsynth::IScriptEnvironment;
using avxsynth::PClip;
using avxsynth::PLANAR_Y;
using avxsynth::PLANAR_U;
using avxsynth::PLANAR_V;
using avxsynth::PVideoFrame;
using avxsynth::VideoInfo;

#endif

AVSValue __cdecl Create_TurnsTile(AVSValue args, void* user_data, IScriptEnvironment* env)
{

  PClip clip = args[0][0].AsClip(),
        tilesheet = 0;
  if (args[0].ArraySize() > 1)
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

#ifdef TURNSTILE_HOST_AVISYNTH_26

  if (vi.IsYUV() && !vi.IsY8())
    lumaW = 1 << vi.GetPlaneWidthSubsampling(PLANAR_U),
    lumaH = 1 << vi.GetPlaneHeightSubsampling(PLANAR_U);
  else
    lumaW = 1,
    lumaH = 1;

#else

  if (vi.IsYUV())
    lumaW = 2;
  else
    lumaW = 1;

  if (vi.IsYV12())
    lumaH = 2;
  else
    lumaH = 1;

#endif


  // Reading arguments out of order makes me feel icky, but I need this early.
  bool interlaced = args[8].AsBool(false);


  int minTileW = lumaW,
      minTileH = lumaH;
  if (interlaced)
    minTileH *= 2;

  // Reduce each default tile dimension to the greatest size, less than or equal
  // to its starting value, that's a factor of the clip and tilesheet dimension,
  // and a multiple of the minimum tile size.
  while (clipW % dTileW > 0 || sheetW % dTileW > 0 || dTileW % minTileW > 0)
    --dTileW;
  while (clipH % dTileH > 0 || sheetH % dTileH > 0 || dTileH % minTileH > 0)
    --dTileH;

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
  if (!vi.IsSameColorspace(vi2))
      env->ThrowError("TurnsTile: clip and tilesheet must share a colorspace!");

  const char* const interlacedStr = interlaced ? "interlaced " : "";
  const char* const cspStr =  vi.IsRGB() ?    "RGB" :
                              vi.IsYUY2() ?   "YUY2" :
                              vi.IsYV12() ?   "YV12" :
#ifdef TURNSTILE_HOST_AVISYNTH_26
                              vi.IsYV24() ?   "YV24" :
                              vi.IsYV16() ?   "YV16" :
                              vi.IsYV411() ?  "YV411" :
                              vi.IsY8() ?     "Y8" :
#endif
                              interlaced ?    "" :
                                              "this";

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


  int maxTileW = TurnsTile::gcf(clipW, sheetW),
      maxTileH = TurnsTile::gcf(clipH, sheetH);

  // These two errors, unlike the two above and the two below, don't mention
  // anything about interlacing or colorspace since the check is performed based
  // only on the greatest common factor of the clip and tilesheet width/height.
  if (tileW > maxTileW)
    env->ThrowError(
      "TurnsTile: For this input, tilew must not exceed %d!",
      maxTileW);

  if (tileH > maxTileH)
    env->ThrowError(
      "TurnsTile: For this input, tileh must not exceed %d!",
      maxTileH);

  if (tileW % minTileW > 0)
    env->ThrowError(
      "TurnsTile: For %s input, tilew must be a multiple of %d!",
      cspStr, minTileW);

  if (tileH % minTileH > 0)
    env->ThrowError(
      "TurnsTile: For %s%s input, tileh must be a multiple of %d!",
      interlacedStr, cspStr, minTileH);

  if (maxTileW % tileW > 0)
    env->ThrowError(
      "TurnsTile: For %s input, tilew must be a factor of %d!",
      cspStr, maxTileW);

  if (maxTileH % tileH > 0)
    env->ThrowError(
      "TurnsTile: For %s%s input, tileh must be a factor of %d!",
      interlacedStr, cspStr, maxTileH);


  int modeMax;
  if (vi.IsYUV())
    modeMax = (lumaW * lumaH) + 2;
  else
    modeMax = vi.BytesFromPixels(1);

  if (mode < 0 || mode > modeMax)
    env->ThrowError(
      "TurnsTile: %s only allows modes 0-%d!",
      cspStr, modeMax);


  if (strcmp(levels, "pc") != 0 && strcmp(levels, "tv") != 0)
    env->ThrowError(
      "TurnsTile: levels must be either \"pc\" or \"tv\"!");


  if (loTile < 0 || loTile > tileIdxMax)
    env->ThrowError(
      "TurnsTile: Valid lotile range is 0-%d!",
      tileIdxMax);

  if (hiTile < 0 || hiTile > tileIdxMax)
    env->ThrowError(
      "TurnsTile: Valid hitile range is 0-%d!",
      tileIdxMax);

  if (loTile > hiTile)
    env->ThrowError(
      "TurnsTile: lotile must not be greater than hitile!");


  if (interlaced) {

    tileH /= 2;
    if (!clip->GetVideoInfo().IsFieldBased())
      clip = env->Invoke("SeparateFields", clip).AsClip();
    if (tilesheet && !tilesheet->GetVideoInfo().IsFieldBased()) {
      tilesheet = env->Invoke("SeparateFields", tilesheet).AsClip();
      vi2 = tilesheet->GetVideoInfo();
    }

  }

  PClip finalClip = new TurnsTile(  clip,
                                    tilesheet,
                                    vi2,
                                    tileW,
                                    tileH,
                                    res,
                                    mode,
                                    levels,
                                    loTile,
                                    hiTile,
                                    env);

  return  interlaced && finalClip->GetVideoInfo().IsFieldBased() ?
            env->Invoke("Weave", finalClip) :
          finalClip;

}



AVSValue __cdecl Create_CLUTer(AVSValue args, void* user_data, IScriptEnvironment* env)
{

  PClip clip = args[0].AsClip(),
        palette = args[1].AsClip();
  VideoInfo vi = clip->GetVideoInfo();


  int paletteFrame = args[2].AsInt(0);


  bool interlaced = args[3].AsBool(false);


  if (!vi.IsSameColorspace(args[1].AsClip()->GetVideoInfo()))
    env->ThrowError("CLUTer: clip and palette must share a colorspace!");


  if (interlaced) {

    const char* const cspStr =  vi.IsRGB() ?  "RGB" :
                                vi.IsYUY2() ? "YUY2" :
                                vi.IsYV12() ? "YV12" :
#ifdef TURNSTILE_HOST_AVISYNTH_26
                                vi.IsYV24() ?   "YV24" :
                                vi.IsYV16() ?   "YV16" :
                                vi.IsYV411() ?  "YV411" :
                                vi.IsY8() ?     "Y8" :
#endif
                                                "";

    int minClipH;
    if (vi.IsYV12())
      minClipH = 4;
    else
      minClipH = 2;

    if (vi.height % minClipH != 0)
      env->ThrowError(
        "CLUTer: %s clip height must be mod %d when interlaced=true!",
        cspStr, minClipH);

    if (!vi.IsFieldBased())
      clip = env->Invoke("SeparateFields", clip).AsClip();

  }


  PClip finalClip = new CLUTer(  clip,
                                 palette,
                                 paletteFrame,
                                 interlaced,
                                 env);

  return  interlaced && finalClip->GetVideoInfo().IsFieldBased() ?
            env->Invoke("Weave", finalClip) :
          finalClip;

}



#ifdef TURNSTILE_HOST_AVISYNTH_26

const AVS_Linkage* AVS_linkage = 0;
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors)
{

  AVS_linkage = vectors;

#else

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{

#endif

  env->AddFunction("CLUTer", "cc[paletteframe]i[interlaced]b",
                             Create_CLUTer, 0);

  env->AddFunction("TurnsTile", "c+[tileW]i[tileH]i[res]i[mode]i[levels]s"
                                "[lotile]i[hitile]i[interlaced]b",
                                Create_TurnsTile, 0);

  return "`TurnsTile' - Mosaic and palette effects";

}