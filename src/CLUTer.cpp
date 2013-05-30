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

#include <cmath>

#include <algorithm>
#include <vector>

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

CLUTer::CLUTer( PClip _child, PClip _palette,
                int _pltFrame, bool _interlaced,
                IScriptEnvironment* env) :
GenericVideoFilter(_child), spp(vi.BytesFromPixels(1)),
PLANAR(vi.IsPlanar()), YUYV(vi.IsYUY2()), BGRA(vi.IsRGB32()), BGR(vi.IsRGB24())
{

  PVideoFrame plt = _palette->GetFrame(_pltFrame,env);
  VideoInfo vi = _palette->GetVideoInfo();


  const unsigned char
    * pltY = plt->GetReadPtr(PLANAR_Y),
    * pltU = plt->GetReadPtr(PLANAR_U),
    * pltV = plt->GetReadPtr(PLANAR_V);

#ifdef TURNSTILE_HOST_AVISYNTH_26

  if (vi.IsYUV() && !vi.IsY8()) {
    lumaW = 1 << vi.GetPlaneWidthSubsampling(PLANAR_U);
    lumaH = 1 << vi.GetPlaneHeightSubsampling(PLANAR_U);
  } else {
    lumaW = 1;
    lumaH = 1;
  }

  if (vi.IsY8()) {
    pltU = 0;
    pltV = 0;
  }

#else

  if (vi.IsYV12()) {
    lumaW = 2;
    lumaH = 2;
  } else if (vi.IsYUY2()) {
    lumaW = 2;
    lumaH = 1;
  } else {
    lumaW = 1;
    lumaH = 1;
  }

#endif


  if (vi.IsPlanar())
    buildPalettePlanar(
      pltY, pltU, pltV, vi.width / lumaW, vi.height / lumaH,
      plt->GetPitch(PLANAR_Y), plt->GetPitch(PLANAR_U));
  else
    buildPalettePacked(pltY, vi.width, vi.height, plt->GetPitch(PLANAR_Y));

}



CLUTer::~CLUTer()
{
}



PVideoFrame __stdcall CLUTer::GetFrame(int n, IScriptEnvironment* env)
{

  PVideoFrame
    src = child->GetFrame(n, env),
    dst = env->NewVideoFrame(vi);


  const unsigned char
    * srcY = src->GetReadPtr(PLANAR_Y),
    * srcU = src->GetReadPtr(PLANAR_U),
    * srcV = src->GetReadPtr(PLANAR_V);

  unsigned char
    * dstY = dst->GetWritePtr(PLANAR_Y),
    * dstU = dst->GetWritePtr(PLANAR_U),
    * dstV = dst->GetWritePtr(PLANAR_V);

  int
    SRC_PITCH_SAMPLES_Y = src->GetPitch(PLANAR_Y),
    SRC_PITCH_SAMPLES_U = src->GetPitch(PLANAR_U),
    DST_PITCH_SAMPLES_Y = dst->GetPitch(PLANAR_Y),
    DST_PITCH_SAMPLES_U = dst->GetPitch(PLANAR_U);
  
#ifdef TURNSTILE_HOST_AVISYNTH_26
  
  if (vi.IsY8()) {
    srcU = 0;
    srcV = 0;
    dstU = 0;
    dstV = 0;
  }
  
#endif


  if (vi.IsPlanar())
    processFramePlanar(
      srcY, srcU, srcV,
      dstY, dstU, dstV,
      vi.width / lumaW, vi.height / lumaH,
      SRC_PITCH_SAMPLES_Y, SRC_PITCH_SAMPLES_U,
      DST_PITCH_SAMPLES_Y, DST_PITCH_SAMPLES_U);
  else
    processFramePacked(
      srcY, dstY, vi.width, vi.height,
      SRC_PITCH_SAMPLES_Y, DST_PITCH_SAMPLES_Y);

  return dst;

}



void CLUTer::buildPalettePacked(
  const unsigned char* pltp,
  const int PLT_WIDTH, const int PLT_HEIGHT,
  const int PLT_PITCH_SAMPLES)
{

  std::vector<int> palette;

  for (int h = 0; h != PLT_HEIGHT; ++h) {

    int pltLine = PLT_PITCH_SAMPLES * h;

    for (int w = 0; w != PLT_WIDTH; w += lumaW) {

      int sample = w * spp;
      int pltOfs = pltLine + sample;

      if (YUYV) {    

        int y1 = *(pltp + pltOfs),
            u =  *(pltp + pltOfs + 1),
            y2 = *(pltp + pltOfs + 2),
            v =  *(pltp + pltOfs + 3);

        palette.push_back((y1 << 16) | (u << 8) | v);
        palette.push_back((y2 << 16) | (u << 8) | v);

      } else {

        int b = *(pltp + pltOfs),
            g = *(pltp + pltOfs + 1),
            r = *(pltp + pltOfs + 2);

        palette.push_back((r << 16) | (g << 8) | b);

      }

    }

  }

  fillComponentVectors(&palette);

}



void CLUTer::processFramePacked(
  const unsigned char* srcp, unsigned char* dstp,
  const int SRC_WIDTH, const int SRC_HEIGHT,
  const int SRC_PITCH_SAMPLES, const int DST_PITCH_SAMPLES)
{

  for (int h = 0; h < SRC_HEIGHT; ++h) {

    int srcLine = SRC_PITCH_SAMPLES * h,
        dstLine = DST_PITCH_SAMPLES * h;

    for (int w = 0; w < SRC_WIDTH; w += lumaW) {

      int sample = w * spp;

      int srcOfs = srcLine + sample,
          dstOfs = dstLine + sample;

      if (YUYV) {

        unsigned char y = *(srcp + srcOfs),
                      u = *(srcp + srcOfs + 1),
                      v = *(srcp + srcOfs + 3);

        int packed = (y << 16) | (u << 8) | v;

        *(dstp + dstOfs) = vecYR[packed];
        *(dstp + dstOfs + 1) = vecUG[packed];
        *(dstp + dstOfs + 2) = vecYR[packed];
        *(dstp + dstOfs + 3) = vecVB[packed];

      } else {

        unsigned char b = *(srcp + srcOfs),
                      g = *(srcp + srcOfs + 1),
                      r = *(srcp + srcOfs + 2);

        int packed = (r << 16) | (g << 8) | b;

        *(dstp + dstOfs) = vecVB[packed];
        *(dstp + dstOfs + 1) = vecUG[packed];
        *(dstp + dstOfs + 2) = vecYR[packed];

      }

    }

  }

}



void CLUTer::buildPalettePlanar(
  const unsigned char* srcY,
  const unsigned char* srcU,
  const unsigned char* srcV,
  const int PLT_WIDTH_U, const int PLT_HEIGHT_U,
  const int PLT_PITCH_SAMPLES_Y, const int PLT_PITCH_SAMPLES_U)
{

  std::vector<int> palette;

  for (int h = 0; h != PLT_HEIGHT_U; ++h) {

    int srcLineY = PLT_PITCH_SAMPLES_Y * h * lumaH,
        srcLineU = PLT_PITCH_SAMPLES_U * h;

    for (int w = 0; w != PLT_WIDTH_U; ++w) {

      int sampleY = w * lumaW,
          sampleU = w;

      int srcOfsY = srcLineY + sampleY,
          srcOfsU = srcLineU + sampleU;

      unsigned char
        u = 0,
        v = 0;

      if (srcU)
        u = *(srcU + srcOfsU);
      if (srcV)
        v = *(srcV + srcOfsU);

      for (int i = 0; i < lumaH; ++i) {

        for (int j = 0; j < lumaW; ++j) {

          unsigned char y = *(srcY + srcOfsY + (PLT_PITCH_SAMPLES_Y * i) + j);
          palette.push_back((y << 16) | (u << 8) | v);

        }

      }

    }

  }

  fillComponentVectors(&palette);

}



void CLUTer::processFramePlanar(
  const unsigned char* srcY,
  const unsigned char* srcU,
  const unsigned char* srcV,
  unsigned char* dstY,
  unsigned char* dstU,
  unsigned char* dstV,
  const int SRC_WIDTH_U, const int SRC_HEIGHT_U,
  const int SRC_PITCH_SAMPLES_Y, const int SRC_PITCH_SAMPLES_U,
  const int DST_PITCH_SAMPLES_Y, const int DST_PITCH_SAMPLES_U)
{

  for (int h = 0; h != SRC_HEIGHT_U; ++h) {

    int srcLineY = SRC_PITCH_SAMPLES_Y * h * lumaH,
        dstLineY = DST_PITCH_SAMPLES_Y * h * lumaH;

    int srcLineU = SRC_PITCH_SAMPLES_U * h,
        dstLineU = DST_PITCH_SAMPLES_U * h;

    for (int w = 0; w != SRC_WIDTH_U; ++w) {

      int sampleY = w * lumaW,
          sampleU = w;

      int srcOfsY = srcLineY + sampleY,
          srcOfsU = srcLineU + sampleU;

      int dstOfsY = dstLineY + sampleY,
          dstOfsU = dstLineU + sampleU;

      unsigned char y = *(srcY + srcOfsY),
                    u = 0,
                    v = 0;

      if (srcU)
        u = *(srcU + srcOfsU);
      if (srcV)
        v = *(srcV + srcOfsU);

      int packed = (y << 16) | (u << 8) | v;

      // TurnsTile can get away with simply zeroing its tileW_U and tileH_U
      // members for Y8, since its fillTile function skips copying data if the
      // height is zero. CLUTer can't use such a copy function, so I need these
      // conditions to only write U and V if the pointers are valid.
      if (dstU)
        *(dstU + dstOfsU) = vecUG[packed];
      if (dstV)
        *(dstV + dstOfsU) = vecVB[packed];

      // I set each luma component in the macropixel to the same value, as
      // otherwise it'd be possible to end up with colors in the output that
      // aren't in the palette. The only solution I can think of would involve
      // a little too much block-by-block calculation for my taste, and
      // wouldn't be worth the effort. CLUTer is, after all, meant to be used
      // with TurnsTile, which will typically involve tiles large enough to
      // hide my little shortcut.
      for (int i = 0; i < lumaH; ++i)
        for (int j = 0; j < lumaW; ++j)
          *(dstY + dstOfsY + (DST_PITCH_SAMPLES_Y * i) + j) = vecYR[packed];

    }

  }

}



void CLUTer::fillComponentVectors(std::vector<int>* plt)
{

  // Adding all colors from the input palette to the palette vector, then
  // sorting it and stripping out the duplicate values, is frighteningly fast,
  // and handily beats the std::find method I'd used previously.
  std::sort(plt->begin(), plt->end());
  plt->erase(std::unique(plt->begin(), plt->end()), plt->end());

  // All unique colors have been read from the input, and the palette's been
  // loaded; now it's time to find the closest match for each possible output.
  // This is another of my naive brute force techniques, but it works, and until
  // I get smarter I'll take working over efficient.
  for (int i = 0; i < 16777216; ++i) {

    int inYR = (i >> 16) & 255,
        inUG = (i >> 8) & 255,
        inVB = i & 255;

    int base = (*plt)[0];

    int pltYR = (base >> 16) & 255,
        pltUG = (base >> 8) & 255,
        pltVB = base & 255;

    // For my use, the sum of absolute differences provides the same results
    // as the Euclidean distance approach I'd been using; I'd implemented that
    // incompletely anyway, and it worked well enough, so I have no qualms
    // using an even simpler, faster technique.
    int sumPrev = abs(inYR - pltYR) +
                  abs(inUG - pltUG) +
                  abs(inVB - pltVB);

    int outInt = base;

    for (std::vector<int>::iterator j = plt->begin(); j != plt->end(); ++j) {

      int pltInt = *j;

      pltYR = (pltInt >> 16) & 255;
      pltUG = (pltInt >> 8) & 255;
      pltVB = pltInt & 255;

      int sumCur = abs(inYR - pltYR) +
                   abs(inUG - pltUG) +
                   abs(inVB - pltVB);

      if (sumCur < sumPrev) {
        sumPrev = sumCur;
        outInt = pltInt;
      }

    }

    vecYR.push_back((outInt >> 16) & 255);
    vecUG.push_back((outInt >> 8) & 255);
    vecVB.push_back(outInt & 255);

  }

}