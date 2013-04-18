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
  paletteGen(pltSrc,pltVi,env);

}



CLUTer::~CLUTer()
{
}



PVideoFrame __stdcall CLUTer::GetFrame(int n, IScriptEnvironment* env)
{

  return vi.IsPlanar() ?  GetFramePlanar(n, env) :
                          GetFrameInterleaved(n, env);

}



PVideoFrame __stdcall CLUTer::GetFrameInterleaved(int n, IScriptEnvironment* env)
{

  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);

  const unsigned char* srcp = src->GetReadPtr();
  unsigned char* dstp =       dst->GetWritePtr();

  const int
    SRC_PITCH =   src->GetPitch(),
    DST_PITCH =   dst->GetPitch();

  int wStep = vi.IsRGB() ? 1 : 2;

  for (int h = 0; h < vi.height; ++h) {

    int srcLine = SRC_PITCH * h,
        dstLine = DST_PITCH * h;
    
    for (int w = 0; w < vi.width; w += wStep) {

      int wBytes = w * bytesPerPixel;

      int srcOffset = srcLine + wBytes,
          dstOffset = dstLine + wBytes;

      if (vi.IsRGB()) {

        unsigned char b = *(srcp + srcOffset),
                      g = *(srcp + srcOffset + 1),
                      r = *(srcp + srcOffset + 2);

        int sheetDec = ((r << 16) | (g << 8)) | b;

        *(dstp + dstOffset) =      vecB[sheetDec];
        *(dstp + dstOffset + 1) =  vecG[sheetDec];
        *(dstp + dstOffset + 2) =  vecR[sheetDec];
        *(dstp + dstOffset + 3) =  *(srcp + srcOffset + 3);

      } else {

        // To avoid creating colors in the output that aren't in the palette, I
        // use the same Y value for Y1 and Y2, so I don't need to read Y2.
        unsigned char y1 =  *(srcp + srcOffset),
                      u =   *(srcp + srcOffset + 1),
                      v =   *(srcp + srcOffset + 3);

        int sheetDec = ((y1 << 16) | (u << 8)) | v;

        *(dstp + dstOffset) =     vecY[sheetDec];
        *(dstp + dstOffset + 1) = vecU[sheetDec];
        *(dstp + dstOffset + 2) = vecY[sheetDec];
        *(dstp + dstOffset + 3) = vecV[sheetDec];

      }
      
    }

  }

  return dst;

}



PVideoFrame __stdcall CLUTer::GetFramePlanar(int n, IScriptEnvironment* env)
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
    DST_PITCH_Y = dst->GetPitch(PLANAR_Y);

  const int
    SRC_WIDTH_UV =  src->GetRowSize(PLANAR_U),
    SRC_HEIGHT_UV = src->GetHeight(PLANAR_U),
    SRC_PITCH_UV =  src->GetPitch(PLANAR_U),
    DST_PITCH_UV =  dst->GetPitch(PLANAR_U);

  for (int h = 0; h != SRC_HEIGHT_UV; ++h) {

    int srcLineY = SRC_PITCH_Y * h * 2,
        dstLineY = DST_PITCH_Y * h * 2;

    int srcLineUV = SRC_PITCH_UV * h,
        dstLineUV = DST_PITCH_UV * h;
    
    for (int w = 0; w != SRC_WIDTH_UV; ++w) {

      int curSampleY =  w * 2,
          curSampleUV = w;

      int srcOffsetY =  srcLineY + curSampleY,
          srcOffsetUV = srcLineUV + curSampleUV;

      int dstOffsetY =  dstLineY + curSampleY,
          dstOffsetUV = dstLineUV + curSampleUV;
    
      unsigned char y = *(srcY + srcOffsetY),
                    u = *(srcU + srcOffsetUV),
                    v = *(srcV + srcOffsetUV);
    
      int sheetDec = ((y << 16) | (u << 8)) | v;
    
      *(dstU + dstOffsetUV) = vecU[sheetDec];
      *(dstV + dstOffsetUV) = vecV[sheetDec];

      // I set each luma component in the 2x2 block to the same value, as
      // otherwise it'd be possible to end up with colors in the output that
      // aren't in the palette. The only solution I can think of would involve
      // a little too much block-by-block calculation for my taste, and
      // wouldn't be worth the effort. CLUTer is, after all, meant to be used
      // with TurnsTile, which will typically involve tiles large enough to
      // hide my little shortcut.
      unsigned char yOut = vecY[sheetDec];

      *(dstY + dstOffsetY) =                   yOut;
      *(dstY + dstOffsetY + 1) =               yOut;
      *(dstY + dstOffsetY + DST_PITCH_Y) =     yOut;
      *(dstY + dstOffsetY + DST_PITCH_Y + 1) = yOut;

    }

  }

  return dst;

}



void CLUTer::paletteGen(PVideoFrame pltSrc, VideoInfo pltVi, IScriptEnvironment* env)
{
  
  const unsigned char* pltp = pltSrc->GetReadPtr();
  const int PLT_PITCH =       pltSrc->GetPitch();
  
  int rgbInt = 0,
      yuvInt1 = 0,
      yuvInt2 = 0;
  
  vector<int> pltMain;

  int pltH = pltSrc->GetHeight(),
      pltW = pltSrc->GetRowSize() / bytesPerPixel;

  if (pltVi.IsRGB()) {

    // No concern for upside down RGB here, as with TurnsTile's "tileIdxY",
    // since I'm just flying through every pixel in the frame and pushing
    // each unique value onto the palette.
    for (int h = 0; h != pltH; ++h) {

      int pltLine = PLT_PITCH * h;

      for (int w = 0; w != pltW; ++w) {
        
        int wBytes = w * bytesPerPixel;
        int pltOffset = pltLine + wBytes;

        int b = *(pltp + pltOffset),
            g = *(pltp + pltOffset + 1),
            r = *(pltp + pltOffset + 2);

        rgbInt = ((r << 16) | (g << 8)) | b;

        if (find(pltMain.begin(), pltMain.end(), rgbInt) == pltMain.end())
          pltMain.push_back(rgbInt);

      }

    }

  } else if (pltVi.IsYUY2()) {

    for (int h = 0; h != pltH; ++h) {

      int pltLine = PLT_PITCH * h;

      for (int w = 0; w != pltW; w += 2) {

        int wBytes = w * bytesPerPixel;
        int pltOffset = pltLine + wBytes;

        int y1 =  *(pltp + pltOffset),
            u =   *(pltp + pltOffset + 1),
            y2 =  *(pltp + pltOffset + 2),
            v =   *(pltp + pltOffset + 3);

        yuvInt1 = ((y1 << 16) | (u << 8)) | v;
        yuvInt2 = ((y2 << 16) | (u << 8)) | v;

        if (find(pltMain.begin(), pltMain.end(), yuvInt1) == pltMain.end())
          pltMain.push_back(yuvInt1);
        
        if (find(pltMain.begin(), pltMain.end(), yuvInt2) == pltMain.end())
          pltMain.push_back(yuvInt2);

      }

    }

  } else if (pltVi.IsYV12()) {

    PVideoFrame src = pltSrc;
    
    const unsigned char
      * srcY = src->GetReadPtr(PLANAR_Y),
      * srcU = src->GetReadPtr(PLANAR_U),
      * srcV = src->GetReadPtr(PLANAR_V);
  
    const int
      SRC_PITCH_Y = src->GetPitch(PLANAR_Y);

    const int
      SRC_WIDTH_UV =  src->GetRowSize(PLANAR_U),
      SRC_HEIGHT_UV = src->GetHeight(PLANAR_U),
      SRC_PITCH_UV =  src->GetPitch(PLANAR_U);
    
    for (int h = 0; h != SRC_HEIGHT_UV; ++h) {

      int srcLineY =  SRC_PITCH_Y * h * 2,
          srcLineUV = SRC_PITCH_UV * h;
    
      for (int w = 0; w != SRC_WIDTH_UV; ++w) {
    
        int curSampleY =  w * 2,
            curSampleUV = w;
                       
        int srcOffsetY =  srcLineY + curSampleY,
            srcOffsetUV = srcLineUV + curSampleUV;

        unsigned char
          u = *(srcU + srcOffsetUV),
          v = *(srcV + srcOffsetUV);

        // Assumes progressive palette chroma, for simplicity; the actual
        // paletting, however, does take interlacing into account.
        unsigned char                                   // 2x2 luma values
          y1 = *(srcY + srcOffsetY),                    // Top left
          y2 = *(srcY + srcOffsetY + 1),                // Top right
          y3 = *(srcY + srcOffsetY + SRC_PITCH_Y),      // Bottom left
          y4 = *(srcY + srcOffsetY + SRC_PITCH_Y + 1);  // Bottom right

        int
          yuvInt1 = ((y1 << 16) | (u << 8)) | v,
          yuvInt2 = ((y2 << 16) | (u << 8)) | v,
          yuvInt3 = ((y3 << 16) | (u << 8)) | v,
          yuvInt4 = ((y4 << 16) | (u << 8)) | v;

        if (find(pltMain.begin(), pltMain.end(), yuvInt1) == pltMain.end())
          pltMain.push_back(yuvInt1);
        
        if (find(pltMain.begin(), pltMain.end(), yuvInt2) == pltMain.end())
          pltMain.push_back(yuvInt2);

        if (find(pltMain.begin(), pltMain.end(), yuvInt3) == pltMain.end())
          pltMain.push_back(yuvInt3);
        
        if (find(pltMain.begin(), pltMain.end(), yuvInt4) == pltMain.end())
          pltMain.push_back(yuvInt4);
              
      }
           
    }

  }
  
  // For a while, during development of version 0.3.0, this value was computed
  // from a pair of defines; I'm sure the pros out there would have found it
  // adorable, but the novelty of such ornate, unnecessary, and potentially
  // dangerous future proofing wore off, and I did eventually wise up, so now
  // it's hardcoded.
  int totalColors = 16777216;

  // All unique colors have been read from the input, and the palette's been
  // loaded; now it's time to find the closest match for each possible output.
  // This is another of my naive brute force techniques, but it works, and until
  // I get smarter I'll take working over efficient.
  if (pltVi.IsRGB()) {

    for (int i = 0; i < totalColors; ++i) {

      int blueIn =  i & 255,
          greenIn = (i >> 8) & 255,
          redIn =   (i >> 16) & 255;

      int base = pltMain[0];

      int pltB = base & 255,
          pltG = (base >> 8) & 255,
          pltR = (base >> 16) & 255;

      int diffB = abs(blueIn - pltB),
          diffG = abs(greenIn - pltG),
          diffR = abs(redIn - pltR);

      // The so-called "Euclidean Distance" between colors doesn't quite model
      // the human eye/brain, to say the least, but it's simple to implement,
      // and for a toy like CLUTer it gets close enough. Perhaps XYZ/L*a*b*
      // computations will come into the picture in the future, who knows?
      int euclidRef = (diffB * diffB) +
                      (diffG * diffG) +
                      (diffR * diffR);

      int outRgbInt = base;

      for (vector<int>::iterator j = pltMain.begin(); j != pltMain.end(); ++j) {
      
        int pltInt = *j;
      
        pltB = pltInt & 255;
        pltG = (pltInt >> 8) & 255;
        pltR = (pltInt >> 16) & 255;

        diffB = abs(blueIn - pltB);
        diffG = abs(greenIn - pltG);
        diffR = abs(redIn - pltR);

        int euclidCur = (diffB * diffB) +
                        (diffG * diffG) +
                        (diffR * diffR);

        if (euclidCur < euclidRef) {
          euclidRef = euclidCur;
          outRgbInt = pltInt;
        }

      }

      vecB.push_back( outRgbInt & 255 );
      vecG.push_back( (outRgbInt >> 8) & 255 );
      vecR.push_back( (outRgbInt >> 16) & 255 );

    }
  
  } else {

    for (int i = 0; i < totalColors; ++i) {
    
      int yIn = (i >> 16) & 255,
          uIn = (i >> 8) & 255,
          vIn = i & 255;
    
      int base = pltMain[0];

      int pltY = (base >> 16) & 255,
          pltU = (base >> 8) & 255,
          pltV = base & 255;

      int diffY = abs(yIn - pltY),
          diffU = abs(uIn - pltU),
          diffV = abs(vIn - pltV);

      int euclidRef = (diffY * diffY) +
                      (diffU * diffU) +
                      (diffV * diffV);

      int outYuvInt = base;

      for (vector<int>::iterator j = pltMain.begin(); j != pltMain.end(); ++j) {

        int pltInt = *j;
      
        pltY = (pltInt >> 16) & 255;
        pltU = (pltInt >> 8) & 255;
        pltV = pltInt & 255;
        
        diffY = abs(yIn - pltY);
        diffU = abs(uIn - pltU);
        diffV = abs(vIn - pltV);

        int euclidCur = (diffY * diffY) +
                        (diffU * diffU) +
                        (diffV * diffV);

        if (euclidCur < euclidRef) {
          euclidRef = euclidCur;
          outYuvInt = pltInt;
        }

      }

      vecY.push_back( (outYuvInt >> 16) & 255 );
      vecU.push_back( (outYuvInt >> 8) & 255 );
      vecV.push_back( outYuvInt & 255 );      
    
    }

  }

}