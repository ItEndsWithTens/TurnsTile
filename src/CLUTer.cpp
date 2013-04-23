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
#include <Windows.h>

#include <algorithm>
#include <vector>

#include "avisynth.h"

using std::vector;



CLUTer::CLUTer( PClip _child, PClip _palette,
                int _pltFrame, bool _interlaced,
                IScriptEnvironment* env) :
GenericVideoFilter(_child)
{

  bytesPerPixel = vi.IsRGB32() ?  4 :
                  vi.IsRGB24() ?  3 :
                  vi.IsYUY2() ?   2 :
                  1;

  PVideoFrame pltSrc = _palette->GetFrame(_pltFrame,env);
  VideoInfo pltVi = _palette->GetVideoInfo();


  const unsigned char
    * pltY = pltSrc->GetReadPtr(PLANAR_Y),
    * pltU = pltSrc->GetReadPtr(PLANAR_U),
    * pltV = pltSrc->GetReadPtr(PLANAR_V);

  if (pltVi.IsYV12())
    lumaW = 2, lumaH = 2;
  else if (pltVi.IsYUY2())
    lumaW = 2, lumaH = 1;
  else
    lumaW = 1, lumaH = 1;


  if (pltVi.IsPlanar())
    buildPalettePlanar(
      pltY, pltU, pltV, pltVi.width / lumaW, pltVi.height / lumaH,
      pltSrc->GetPitch(PLANAR_Y), pltSrc->GetPitch(PLANAR_U));
  else
    if (pltVi.IsYUY2())
      buildPaletteYuyv(
        pltY, pltVi.width, pltVi.height, pltSrc->GetPitch(PLANAR_Y));
    else
      buildPaletteBgr(
        pltY, pltVi.width, pltVi.height, pltSrc->GetPitch(PLANAR_Y));

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

  const int
    SRC_PITCH_Y = src->GetPitch(PLANAR_Y),
    SRC_PITCH_UV = src->GetPitch(PLANAR_U),
    DST_PITCH_Y = dst->GetPitch(PLANAR_Y),
    DST_PITCH_UV = dst->GetPitch(PLANAR_U);


  if (vi.IsPlanar())
    processFramePlanar(
      srcY, srcU, srcV,
      dstY, dstU, dstV,
      vi.width / lumaW, vi.height / lumaH,
      SRC_PITCH_Y, SRC_PITCH_UV, DST_PITCH_Y, DST_PITCH_UV);
  else
    if (vi.IsYUY2())
      processFrameYuyv(
        srcY, dstY, vi.width, vi.height, SRC_PITCH_Y, DST_PITCH_Y);
    else
      processFrameBgr(
        srcY, dstY, vi.width, vi.height, SRC_PITCH_Y, DST_PITCH_Y);

  return dst;

}



void CLUTer::processFrameBgr(
  const unsigned char* srcp, unsigned char* dstp,
  int width, int height,
  const int SRC_PITCH, const int DST_PITCH)
{

  for (int h = 0; h < height; ++h) {

    int srcLine = SRC_PITCH * h,
        dstLine = DST_PITCH * h;

    for (int w = 0; w < width; ++w) {

      int wBytes = w * bytesPerPixel;

      int srcOffset = srcLine + wBytes,
          dstOffset = dstLine + wBytes;

      unsigned char b = *(srcp + srcOffset),
                    g = *(srcp + srcOffset + 1),
                    r = *(srcp + srcOffset + 2);

      int packed = ((r << 16) | (g << 8)) | b;

      *(dstp + dstOffset) = vecBV[packed];
      *(dstp + dstOffset + 1) = vecGU[packed];
      *(dstp + dstOffset + 2) = vecRY[packed];

    }

  }

}



void CLUTer::processFrameYuyv(
  const unsigned char* srcp, unsigned char* dstp,
  int width, int height,
  const int SRC_PITCH, const int DST_PITCH)
{

  for (int h = 0; h < height; ++h) {

    int srcLine = SRC_PITCH * h,
        dstLine = DST_PITCH * h;

    for (int w = 0; w < width; w += 2) {

      int wBytes = w * bytesPerPixel;

      int srcOffset = srcLine + wBytes,
          dstOffset = dstLine + wBytes;

      unsigned char y = *(srcp + srcOffset),
                    u = *(srcp + srcOffset + 1),
                    v = *(srcp + srcOffset + 3);

      int packed = ((y << 16) | (u << 8)) | v;

      *(dstp + dstOffset) = vecRY[packed];
      *(dstp + dstOffset + 1) = vecGU[packed];
      *(dstp + dstOffset + 2) = vecRY[packed];
      *(dstp + dstOffset + 3) = vecBV[packed];

    }

  }

}



void CLUTer::processFramePlanar(
  const unsigned char* srcY,
  const unsigned char* srcU,
  const unsigned char* srcV,
  unsigned char* dstY,
  unsigned char* dstU,
  unsigned char* dstV,
  const int SRC_WIDTH_UV, const int SRC_HEIGHT_UV,
  const int SRC_PITCH_Y, const int SRC_PITCH_UV,
  const int DST_PITCH_Y, const int DST_PITCH_UV)
{

  for (int h = 0; h != SRC_HEIGHT_UV; ++h) {

    int srcLineY = SRC_PITCH_Y * h * lumaH,
        dstLineY = DST_PITCH_Y * h * lumaH;

    int srcLineUV = SRC_PITCH_UV * h,
        dstLineUV = DST_PITCH_UV * h;

    for (int w = 0; w != SRC_WIDTH_UV; ++w) {

      int macropixelY = w * lumaW,
          macropixelUV = w;

      int srcOffsetY = srcLineY + macropixelY,
          srcOffsetUV = srcLineUV + macropixelUV;

      int dstOffsetY = dstLineY + macropixelY,
          dstOffsetUV = dstLineUV + macropixelUV;

      unsigned char y = *(srcY + srcOffsetY),
                    u = *(srcU + srcOffsetUV),
                    v = *(srcV + srcOffsetUV);

      int packed = ((y << 16) | (u << 8)) | v;

      *(dstU + dstOffsetUV) = vecGU[packed];
      *(dstV + dstOffsetUV) = vecBV[packed];

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



void CLUTer::buildPaletteBgr(
  const unsigned char* pltp, int width, int height, const int PLT_PITCH)
{
  
  vector<int> pltMain;

  for (int h = 0; h != height; ++h) {

    int pltLine = PLT_PITCH * h;

    for (int w = 0; w != width; w += lumaW) {
        
      int wBytes = w * bytesPerPixel;
      int pltOffset = pltLine + wBytes;

      int b = *(pltp + pltOffset),
          g = *(pltp + pltOffset + 1),
          r = *(pltp + pltOffset + 2);

      pltMain.push_back((r << 16) | (g << 8) | b);

    }

  }

  fillComponentVectors(&pltMain);

}



void CLUTer::buildPaletteYuyv(
  const unsigned char* pltp, int width, int height, const int PLT_PITCH)
{

  vector<int> pltMain;

  for (int h = 0; h != height; ++h) {

    int pltLine = PLT_PITCH * h;

    for (int w = 0; w != width; w += lumaW) {
        
      int wBytes = w * bytesPerPixel;
      int pltOffset = pltLine + wBytes;

      int y1 =  *(pltp + pltOffset),
          u =   *(pltp + pltOffset + 1),
          y2 =  *(pltp + pltOffset + 2),
          v =   *(pltp + pltOffset + 3);

      pltMain.push_back((y1 << 16) | (u << 8) | v),
      pltMain.push_back((y2 << 16) | (u << 8) | v);

    }

  }

  fillComponentVectors(&pltMain);

}



void CLUTer::buildPalettePlanar(
  const unsigned char* srcY,
  const unsigned char* srcU,
  const unsigned char* srcV,
  int widthUV, int heightUV, const int PLT_PITCH_Y, const int PLT_PITCH_UV)
{

  vector<int> pltMain;

  for (int h = 0; h != heightUV; ++h) {

    int srcLineY =  PLT_PITCH_Y * h * lumaH,
        srcLineUV = PLT_PITCH_UV * h;
    
    for (int w = 0; w != widthUV; ++w) {
    
      int curSampleY =  w * lumaW,
          curSampleUV = w;
                       
      int srcOffsetY =  srcLineY + curSampleY,
          srcOffsetUV = srcLineUV + curSampleUV;

      unsigned char
        u = *(srcU + srcOffsetUV),
        v = *(srcV + srcOffsetUV);

      for (int i = 0; i < lumaH; ++i) {

        for (int j = 0; j < lumaW; ++j) {

          unsigned char y = *(srcY + srcOffsetY + (PLT_PITCH_Y * i) + j);
          pltMain.push_back((y << 16) | (u << 8) | v);

        }

      }

    }
           
  }
  
  fillComponentVectors(&pltMain);

}



void CLUTer::fillComponentVectors(std::vector<int>* palette)
{

  // Adding all colors from the input palette to the pltMain vector, then
  // sorting it and stripping out the duplicate values, is frighteningly fast,
  // and handily beats the std::find method I'd used previously.
  std::sort(palette->begin(), palette->end());
  palette->erase(std::unique(palette->begin(), palette->end()), palette->end());

  // All unique colors have been read from the input, and the palette's been
  // loaded; now it's time to find the closest match for each possible output.
  // This is another of my naive brute force techniques, but it works, and until
  // I get smarter I'll take working over efficient.
  for (int i = 0; i < 16777216; ++i) {

    int inRY = (i >> 16) & 255,
        inGU = (i >> 8) & 255,
        inBV = i & 255;

    int base = (*palette)[0];

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

    for (vector<int>::iterator j = palette->begin(); j != palette->end(); ++j) {
      
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

    vecRY.push_back( (outInt >> 16) & 255 );
    vecGU.push_back( (outInt >> 8) & 255 );
    vecBV.push_back( outInt & 255 );

  }

}