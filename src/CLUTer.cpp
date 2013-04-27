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



CLUTer::CLUTer( PClip _child, PClip _palette,
                int _pltFrame, bool _interlaced,
                IScriptEnvironment* env) :
GenericVideoFilter(_child), samplesPerPixel(vi.BytesFromPixels(1))
{

  PVideoFrame plt = _palette->GetFrame(_pltFrame,env);
  VideoInfo vi = _palette->GetVideoInfo();


  const unsigned char
    * pltY = plt->GetReadPtr(PLANAR_Y),
    * pltU = plt->GetReadPtr(PLANAR_U),
    * pltV = plt->GetReadPtr(PLANAR_V);

#ifdef TURNSTILE_HOST_AVISYNTH_26

  if (vi.IsYUV() && !vi.IsY8())
    lumaW = 1 << vi.GetPlaneWidthSubsampling(PLANAR_U),
    lumaH = 1 << vi.GetPlaneHeightSubsampling(PLANAR_U);
  else
    lumaW = 1, lumaH = 1;

#else

  if (vi.IsYV12())
    lumaW = 2, lumaH = 2;
  else if (vi.IsYUY2())
    lumaW = 2, lumaH = 1;
  else
    lumaW = 1, lumaH = 1;

#endif


  if (vi.IsPlanar())
    buildPalettePlanar(
      pltY, pltU, pltV, vi.width / lumaW, vi.height / lumaH,
      plt->GetPitch(PLANAR_Y), plt->GetPitch(PLANAR_U));
  else
    if (vi.IsYUY2())
      buildPaletteYuyv(pltY, vi.width, vi.height, plt->GetPitch(PLANAR_Y));
    else
      buildPaletteBgr(pltY, vi.width, vi.height, plt->GetPitch(PLANAR_Y));

}



CLUTer::~CLUTer()
{
}



PVideoFrame __stdcall CLUTer::GetFrame(int n, IScriptEnvironment* env)
{

  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);


  const unsigned char
    * srcY = src->GetReadPtr(PLANAR_Y),
    * srcU = src->GetReadPtr(PLANAR_U),
    * srcV = src->GetReadPtr(PLANAR_V);

  unsigned char
    * dstY = dst->GetWritePtr(PLANAR_Y),
    * dstU = dst->GetWritePtr(PLANAR_U),
    * dstV = dst->GetWritePtr(PLANAR_V);

  int
    SRC_PITCH_Y = src->GetPitch(PLANAR_Y),
    SRC_PITCH_U = src->GetPitch(PLANAR_U),
    DST_PITCH_Y = dst->GetPitch(PLANAR_Y),
    DST_PITCH_U = dst->GetPitch(PLANAR_U);

  // For handling Y8 clips, without another Avisynth 2.6 ifdef. Asking for read
  // and write pointers to the U and V planes with a Y8 clip gives duplicates of
  // the Y plane pointers, but GetPitch returns zero, so I need to adjust these.
  if (SRC_PITCH_U == 0)
    SRC_PITCH_U = SRC_PITCH_Y;
  if (DST_PITCH_U == 0)
    DST_PITCH_U = DST_PITCH_Y;


  if (vi.IsPlanar())
    processFramePlanar(
      srcY, srcU, srcV,
      dstY, dstU, dstV,
      vi.width / lumaW, vi.height / lumaH,
      SRC_PITCH_Y, SRC_PITCH_U, DST_PITCH_Y, DST_PITCH_U);
  else
    if (vi.IsYUY2())
      processFrameYuyv(
        srcY, dstY, vi.width, vi.height, SRC_PITCH_Y, DST_PITCH_Y);
    else
      processFrameBgr(
        srcY, dstY, vi.width, vi.height, SRC_PITCH_Y, DST_PITCH_Y);

  return dst;

}



void CLUTer::buildPaletteBgr(
  const unsigned char* pltp,
  const int PLT_WIDTH, const int PLT_HEIGHT,
  const int PLT_PITCH)
{

  std::vector<int> palette;

  for (int h = 0; h != PLT_HEIGHT; ++h) {

    int pltLine = PLT_PITCH * h;

    for (int w = 0; w != PLT_WIDTH; w += lumaW) {

      int sample = w * samplesPerPixel;
      int pltOffset = pltLine + sample;

      int b = *(pltp + pltOffset),
          g = *(pltp + pltOffset + 1),
          r = *(pltp + pltOffset + 2);

      palette.push_back((r << 16) | (g << 8) | b);

    }

  }

  fillComponentVectors(&palette);

}



void CLUTer::processFrameBgr(
  const unsigned char* srcp, unsigned char* dstp,
  const int SRC_WIDTH, const int SRC_HEIGHT,
  const int SRC_PITCH, const int DST_PITCH)
{

  for (int h = 0; h < SRC_HEIGHT; ++h) {

    int srcLine = SRC_PITCH * h,
        dstLine = DST_PITCH * h;

    for (int w = 0; w < SRC_WIDTH; ++w) {

      int sample = w * samplesPerPixel;

      int srcOffset = srcLine + sample,
          dstOffset = dstLine + sample;

      unsigned char b = *(srcp + srcOffset),
                    g = *(srcp + srcOffset + 1),
                    r = *(srcp + srcOffset + 2);

      int packed = (r << 16) | (g << 8) | b;

      *(dstp + dstOffset) = vecBV[packed];
      *(dstp + dstOffset + 1) = vecGU[packed];
      *(dstp + dstOffset + 2) = vecRY[packed];

    }

  }

}



void CLUTer::buildPaletteYuyv(
  const unsigned char* pltp,
  const int PLT_WIDTH, const int PLT_HEIGHT,
  const int PLT_PITCH)
{

  std::vector<int> palette;

  for (int h = 0; h != PLT_HEIGHT; ++h) {

    int pltLine = PLT_PITCH * h;

    for (int w = 0; w != PLT_WIDTH; w += lumaW) {

      int sample = w * samplesPerPixel;
      int pltOffset = pltLine + sample;

      int y1 =  *(pltp + pltOffset),
          u =   *(pltp + pltOffset + 1),
          y2 =  *(pltp + pltOffset + 2),
          v =   *(pltp + pltOffset + 3);

      palette.push_back((y1 << 16) | (u << 8) | v),
      palette.push_back((y2 << 16) | (u << 8) | v);

    }

  }

  fillComponentVectors(&palette);

}



void CLUTer::processFrameYuyv(
  const unsigned char* srcp, unsigned char* dstp,
  const int SRC_WIDTH, const int SRC_HEIGHT,
  const int SRC_PITCH, const int DST_PITCH)
{

  for (int h = 0; h < SRC_HEIGHT; ++h) {

    int srcLine = SRC_PITCH * h,
        dstLine = DST_PITCH * h;

    for (int w = 0; w < SRC_WIDTH; w += 2) {

      int sample = w * samplesPerPixel;

      int srcOffset = srcLine + sample,
          dstOffset = dstLine + sample;

      unsigned char y = *(srcp + srcOffset),
                    u = *(srcp + srcOffset + 1),
                    v = *(srcp + srcOffset + 3);

      int packed = (y << 16) | (u << 8) | v;

      *(dstp + dstOffset) = vecRY[packed];
      *(dstp + dstOffset + 1) = vecGU[packed];
      *(dstp + dstOffset + 2) = vecRY[packed];
      *(dstp + dstOffset + 3) = vecBV[packed];

    }

  }

}



void CLUTer::buildPalettePlanar(
  const unsigned char* srcY,
  const unsigned char* srcU,
  const unsigned char* srcV,
  const int PLT_WIDTH_U, const int PLT_HEIGHT_U,
  const int PLT_PITCH_Y, const int PLT_PITCH_U)
{

  std::vector<int> palette;

  for (int h = 0; h != PLT_HEIGHT_U; ++h) {

    int srcLineY = PLT_PITCH_Y * h * lumaH,
        srcLineU = PLT_PITCH_U * h;

    for (int w = 0; w != PLT_WIDTH_U; ++w) {

      int sampleY = w * lumaW,
          sampleU = w;

      int srcOffsetY = srcLineY + sampleY,
          srcOffsetU = srcLineU + sampleU;

      unsigned char
        u = *(srcU + srcOffsetU),
        v = *(srcV + srcOffsetU);

      for (int i = 0; i < lumaH; ++i) {

        for (int j = 0; j < lumaW; ++j) {

          unsigned char y = *(srcY + srcOffsetY + (PLT_PITCH_Y * i) + j);
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
  const int SRC_PITCH_Y, const int SRC_PITCH_U,
  const int DST_PITCH_Y, const int DST_PITCH_U)
{

  for (int h = 0; h != SRC_HEIGHT_U; ++h) {

    int srcLineY = SRC_PITCH_Y * h * lumaH,
        dstLineY = DST_PITCH_Y * h * lumaH;

    int srcLineU = SRC_PITCH_U * h,
        dstLineU = DST_PITCH_U * h;

    for (int w = 0; w != SRC_WIDTH_U; ++w) {

      int sampleY = w * lumaW,
          sampleU = w;

      int srcOffsetY = srcLineY + sampleY,
          srcOffsetU = srcLineU + sampleU;

      int dstOffsetY = dstLineY + sampleY,
          dstOffsetU = dstLineU + sampleU;

      unsigned char y = *(srcY + srcOffsetY),
                    u = *(srcU + srcOffsetU),
                    v = *(srcV + srcOffsetU);

      int packed = (y << 16) | (u << 8) | v;

      *(dstU + dstOffsetU) = vecGU[packed];
      *(dstV + dstOffsetU) = vecBV[packed];

      // I set each luma component in the macropixel to the same value, as
      // otherwise it'd be possible to end up with colors in the output that
      // aren't in the palette. The only solution I can think of would involve
      // a little too much block-by-block calculation for my taste, and
      // wouldn't be worth the effort. CLUTer is, after all, meant to be used
      // with TurnsTile, which will typically involve tiles large enough to
      // hide my little shortcut.
      for (int i = 0; i < lumaH; ++i)
        for (int j = 0; j < lumaW; ++j)
          *(dstY + dstOffsetY + (DST_PITCH_Y * i) + j) = vecRY[packed];

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

    int inRY = (i >> 16) & 255,
        inGU = (i >> 8) & 255,
        inBV = i & 255;

    int base = (*plt)[0];

    int pltRY = (base >> 16) & 255,
        pltGU = (base >> 8) & 255,
        pltBV = base & 255;

    // For my use, the sum of absolute differences provides the same results
    // as the Euclidean distance approach I'd been using; I'd implemented that
    // incompletely anyway, and it worked well enough, so I have no qualms
    // using an even simpler, faster technique.
    int sumPrev = abs(inRY - pltRY) +
                  abs(inGU - pltGU) +
                  abs(inBV - pltBV);

    int outInt = base;

    for (std::vector<int>::iterator j = plt->begin(); j != plt->end(); ++j) {

      int pltInt = *j;

      pltRY = (pltInt >> 16) & 255;
      pltGU = (pltInt >> 8) & 255;
      pltBV = pltInt & 255;

      int sumCur = abs(inRY - pltRY) +
                   abs(inGU - pltGU) +
                   abs(inBV - pltBV);

      if (sumCur < sumPrev) {
        sumPrev = sumCur;
        outInt = pltInt;
      }

    }

    vecRY.push_back((outInt >> 16) & 255);
    vecGU.push_back((outInt >> 8) & 255);
    vecBV.push_back(outInt & 255);

  }

}