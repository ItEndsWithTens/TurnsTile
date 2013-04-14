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



#include "TurnsTile.h"

#include <cmath>
#include <cstring>

#include <algorithm>

#include <Windows.h>

#include "avisynth.h"



TurnsTile::TurnsTile( PClip _child, PClip _tileSheet, VideoInfo _vi2,
                      int _tileW, int _tileH,
                      int _res, int _mode,
                      const char* _levels,
                      int _loTile, int _hiTile,
                      IScriptEnvironment* env) :
GenericVideoFilter(_child), tileSheet(_tileSheet),
tileW(_tileW), tileH(_tileH), mode(_mode),
srcRows(vi.height / tileH), srcCols(vi.width / tileW),
shtRows(_vi2.height / tileH), shtCols(_vi2.width / tileW),
bytesPerPixel(vi.BytesFromPixels(1)), tileBytes(tileW * bytesPerPixel),
host(env)
{
  
  int idxInMin =  strcmp(_levels, "pc") == 0 ? 0 : 16;
  int idxInMax =  strcmp(_levels, "pc") == 0 ? 255 :
                  (vi.IsYUY2() && (mode == 2 || mode == 4)) ||
                  (vi.IsYV12() && (mode == 5 || mode == 6)) ? 240 : 235;

  wStep = vi.IsRGB() ? 1 : 2;

  // An easy way to simulate the look of decreased bit depth; treat 'res'
  // as desired number of bits per component, then cut the output range
  // into as many steps as said number of bits would provide.
  //
  // This has been modified, for version 0.3.0, from previous versions. I now
  // subtract 1 from the result of raising 2.0 to res. Although dividing the
  // output range by the result of the power function would give you as many
  // pieces, your values will be rounded to the boundaries between those pieces,
  // so to speak.
  //
  // A res of 1, for example, is supposed to simulate one bit per component;
  // that is, each of R, G, and B, or Y, U, and V can have one of two possible
  // values. 2.0 to the power of 1 is 2, and 256 / 2 is 128, but rounding to the
  // nearest multiple of 128 will produce three possibilities: 0, 128, and 256.
  // The mod() function caps that 256 at 255, but the problem of three values
  // instead of two remains. I want 0 and 256 to be the two options, and frankly
  // the only solution I was able to work out was the subtraction.
  depthStep = static_cast<int>(ceil(  (_hiTile - _loTile) /
                                      (pow(2.0, _res) - 1.0)  ));

  if (tileSheet) {

    double idxScaleFactor = static_cast<double> (idxInMax - idxInMin) /
                            static_cast<double> (_hiTile - _loTile);

    for (int in = 0; in < 256; ++in) {

      // The proper, generic form of this scaling formula would be:
      //
      // ((number) /
      //   ((inMax - inMin) / (outMax - outMin))) +
      // outMin
      //
      // Using TurnsTile::mod() to round the result is a unique requirement of the
      // 'res' feature I've got in TurnsTile, you don't need it if only scaling.
      int scaled =  static_cast<int>((in / idxScaleFactor) + 0.5) + _loTile;
  
      int out = TurnsTile::mod(scaled, depthStep, _loTile, _hiTile, 0);

      tileIdxLut.push_back(out);

    }

  } else {

    // No need to scale 'in' here, as above, since this only deals with
    // component values (not tile indices), which with 8bpc color will always
    // be less than 256.
    for (int in = 0; in < 256; ++in) {

      unsigned char out = static_cast<unsigned char> (
                            TurnsTile::mod(in, depthStep, _loTile, _hiTile, 0)
                          );

      componentLut.push_back(out);

    }

  }

}



TurnsTile::~TurnsTile()
{
}



PVideoFrame __stdcall TurnsTile::GetFrame(int n, IScriptEnvironment* env)
{

  PVideoFrame
    src = child->GetFrame(n, env),
    sht = 0,
    dst = env->NewVideoFrame(vi);

  const unsigned char
    * srcY = src->GetReadPtr(PLANAR_Y),
    * srcU = src->GetReadPtr(PLANAR_U),
    * srcV = src->GetReadPtr(PLANAR_V),
    * shtY = 0,
    * shtU = 0,
    * shtV = 0;

  unsigned char
    * dstY = dst->GetWritePtr(PLANAR_Y),
    * dstU = dst->GetWritePtr(PLANAR_U),
    * dstV = dst->GetWritePtr(PLANAR_V);

  int
    SRC_PITCH_Y = src->GetPitch(PLANAR_Y),
    SRC_PITCH_UV = src->GetPitch(PLANAR_U),
    SHT_PITCH_Y = 0,
    SHT_PITCH_UV = 0,
    DST_PITCH_Y = dst->GetPitch(PLANAR_Y),
    DST_PITCH_UV = dst->GetPitch(PLANAR_U);

  if (tileSheet) {

    sht = tileSheet->GetFrame(n, env);

    shtY = sht->GetReadPtr(PLANAR_Y),
    shtU = sht->GetReadPtr(PLANAR_U),
    shtV = sht->GetReadPtr(PLANAR_V);

    SHT_PITCH_Y = sht->GetPitch(PLANAR_Y),
    SHT_PITCH_UV = sht->GetPitch(PLANAR_U);

  }

  if (vi.IsPlanar())
    TurnsTile::processFramePlanar(
      srcY, srcU, srcV,
      shtY, shtU, shtV,
      dstY, dstU, dstV,
      SRC_PITCH_Y, SRC_PITCH_UV,
      SHT_PITCH_Y, SHT_PITCH_UV,
      DST_PITCH_Y, DST_PITCH_UV);
  else
    TurnsTile::processFramePacked(
      srcY, shtY, dstY, SRC_PITCH_Y, SHT_PITCH_Y, DST_PITCH_Y);

  return dst;

}



void __stdcall TurnsTile::processFramePacked(
  const unsigned char* srcp, const unsigned char* shtp, unsigned char* dstp,
  const int SRC_PITCH, const int SHT_PITCH, const int DST_PITCH)
{

  for (int row = 0; row < srcRows; ++row) {
  
    int srcRow = SRC_PITCH * row * tileH,
        dstRow = DST_PITCH * row * tileH;

    for (int col = 0; col < srcCols; ++col) {
      
      int curCol = col * tileBytes;
      
      int minTileW = vi.IsYUY2() ? 2 : 1;

      // Be careful with operations like this center offset stuff; early on, I
      // wasn't careful with my parentheses, and had to track down an obnoxious
      // bug that shifted the picture when minimum-size tiles were used.
      int ctrW = ((tileW / 2) / minTileW) * minTileW * bytesPerPixel;
      int ctrH = SRC_PITCH * (tileH / 2);

      int tileCtr = srcRow + curCol + ctrW + ctrH;
      
      int cropLeft = 0,
          cropTop = 0;

      if (tileSheet) {

        int rawIdx = 0;
        
        switch (mode) {

        case 1 :  rawIdx =  *(srcp + tileCtr);     // Y1 / Blue
          break;

        case 2 :  rawIdx =  *(srcp + tileCtr + 1); // U /  Green
          break;

        case 3 :  rawIdx =  *(srcp + tileCtr + 2); // Y2 / Red
          break;

        case 4 :  rawIdx =  *(srcp + tileCtr + 3); // V /  Alpha
          break;

        default :
          vi.IsYUY2() ?
                  rawIdx = (*(srcp + tileCtr) +
                            *(srcp + tileCtr + 2)) / 2 :

                  // As Charles Poynton and countless others can tell you, this
                  // average of R, G, and B is not the same as "brightness". It
                  // is, however, simple, and close enough to achieve pleasing
                  // results for such trivial purposes.
                  rawIdx = (*(srcp + tileCtr) +
                            *(srcp + tileCtr + 1) +
                            *(srcp + tileCtr + 2)) / 3;                    
          break;

        }

        int tileIdx = tileIdxLut[rawIdx];
		
        // If I didn't have to worry about the differences in data storage
        // between RGB and YUV, I would skip declaring tileIdxY and just jump
        // right to cropTop, making it SHT_PITCH * (tileIdx / shtCols) * tileH.
        // Since RGB is "upside down" in memory, however, the desire for
        // consistent tile numbering demands that I create this new variable and
        // then do the subtraction; as a result, the top left tile is number 0
        // regardless of input colorspace.
        int tileIdxY = tileIdx / shtCols;

        if (vi.IsRGB())
          tileIdxY = shtRows - 1 - tileIdxY;

        // Modulo here has the effect of "wrapping around" the horizontal tile
        // count for the sheet you've provided.
        cropLeft = (tileIdx % shtCols) * tileBytes;
        cropTop = SHT_PITCH * tileIdxY * tileH;

      }

      unsigned char* dstTile = dstp + dstRow + curCol;
      const unsigned char* shtTile = shtp + cropTop + cropLeft;

      if (tileSheet) {

        fillTile(dstTile, DST_PITCH, shtTile, SHT_PITCH, tileW, tileH, 0);

      } else {

        unsigned char by = componentLut[*(srcp + tileCtr)],
                      gu = componentLut[*(srcp + tileCtr + 1)],
                      ry = componentLut[*(srcp + tileCtr + 2)],
                      av = componentLut[*(srcp + tileCtr + 3)];

        if (vi.IsRGB32() || vi.IsYUY2()) {

          unsigned int fillVal;
          if (vi.IsRGB32())
            fillVal = (av << 24) | (ry << 16) | (gu << 8) | by;
          else
            fillVal = (av << 24) | (by << 16) | (gu << 8) | by;

          fillTile(
            dstTile, DST_PITCH, static_cast<const unsigned char*>(0), 0,
            tileW, tileH, fillVal);

        } else {

          // For the time being, I'm giving RGB24 its own slow, manual loop,
          // instead of trying to get fillTile to handle a funny stepping
          // sequence for a three byte pixel written four bytes at a time.
          for (int h = 0; h < tileH; ++h) {

            int dstLine = DST_PITCH * h;

            for (int w = 0; w < tileW; ++w) {

              int dstOffset = dstRow + curCol + dstLine + (w * bytesPerPixel);

              for (int i = 0; i < bytesPerPixel; ++i)
                *(dstp + dstOffset + i) = componentLut[*(srcp + tileCtr + i)];

            }

          }

        }

      }
		
    }

  }

}



void __stdcall TurnsTile::processFramePlanar(
  const unsigned char* srcY,
  const unsigned char* srcU,
  const unsigned char* srcV,
  const unsigned char* shtY,
  const unsigned char* shtU,
  const unsigned char* shtV,
  unsigned char* dstY,
  unsigned char* dstU,
  unsigned char* dstV,
  const int SRC_PITCH_Y, const int SRC_PITCH_UV,
  const int SHT_PITCH_Y, const int SHT_PITCH_UV,
  const int DST_PITCH_Y, const int DST_PITCH_UV)
{

  for (int row = 0; row < srcRows; ++row) {

    int srcRowY =   SRC_PITCH_Y * row * tileH,
        srcRowUV =  SRC_PITCH_UV * row * (tileH / 2);

    int dstRowY =   DST_PITCH_Y * row * tileH,
        dstRowUV =  DST_PITCH_UV * row * (tileH / 2);

    for (int col = 0; col < srcCols; ++col) {

      int curColY =   col * tileW,
          curColUV =  col * (tileW / 2);

      unsigned char
          * dstTileY = dstY + dstRowY + curColY,
          * dstTileU = dstU + dstRowUV + curColUV,
          * dstTileV = dstV + dstRowUV + curColUV;

      // As you'll note, in the if (tileSheet) code below I read four Y
      // values for every 2x2 block of YV12. Those four are read relative
      // to the starting position, so I need to start at the top left corner
      // of the block, not the center. Otherwise, y1 through y4, u and v would
      // be read from the wrong locations.
      //
      // Rounding these center offsets to the next lowest multiple of the
      // block size gets me to the proper position. I use / 4 for the U and V
      // centers because I need the center of the tile (/ 2) and for YV12 the
      // U and V plane dimensions are half the Y plane (another / 2).
      int ctrW_Y =  ((tileW / 2) / 2) * 2,
          ctrW_UV = ((tileW / 4) / 4) * 4;
        
      int ctrH_Y =  SRC_PITCH_Y * ((tileH / 2) / 2) * 2,
          ctrH_UV = SRC_PITCH_UV * ((tileH / 4) / 4) * 4;

      int tileCtrY =   srcRowY + curColY + ctrW_Y + ctrH_Y,
          tileCtrUV =  srcRowUV + curColUV + ctrW_UV + ctrH_UV;

      if (tileSheet) {

        int rawIdx = 0;

        unsigned char
          y1 =  *(srcY + tileCtrY),
          y2 =  *(srcY + tileCtrY + 1),
          y3 =  *(srcY + tileCtrY + SRC_PITCH_Y),
          y4 =  *(srcY + tileCtrY + SRC_PITCH_Y + 1),
          u =   *(srcU + tileCtrUV),
          v =   *(srcV + tileCtrUV);

        switch (mode) {

        case 1 : rawIdx =   y1;
          break;

        case 2 : rawIdx =   y2;
          break;

        case 3 : rawIdx =   y3;
          break;

        case 4 : rawIdx =   y4;
          break;

        case 5 : rawIdx =   u;
          break;

        case 6 : rawIdx =   v;
          break;

        default : rawIdx =  (y1 + y2 + y3 + y4) / 4;
          break;

        }

        int tileIdx = tileIdxLut[rawIdx];
		    
        int cropLeftY =  (tileIdx % shtCols) * tileW,
            cropLeftUV = (tileIdx % shtCols) * (tileW / 2),
            cropTopY =  SHT_PITCH_Y * (tileIdx / shtCols) * tileH,
            cropTopUV = SHT_PITCH_UV * (tileIdx / shtCols) * (tileH / 2);

        const unsigned char
          * shtTileY = shtY + cropLeftY + cropTopY,
          * shtTileU = shtU + cropLeftUV + cropTopUV,
          * shtTileV = shtV + cropLeftUV + cropTopUV;

        fillTile(
          dstTileY, SRC_PITCH_Y, shtTileY, SHT_PITCH_Y, tileW, tileH, 0);
        fillTile(
          dstTileU, SRC_PITCH_UV, shtTileU, SHT_PITCH_UV, tileW / 2, tileH / 2, 0);
        fillTile(
          dstTileV, SRC_PITCH_UV, shtTileV, SHT_PITCH_UV, tileW / 2, tileH / 2, 0);

      } else {

        fillTile(
          dstTileY, SRC_PITCH_Y, static_cast<unsigned char*>(0), 0,
          tileW, tileH, static_cast<unsigned char>(componentLut[*(srcY + tileCtrY)]));
        fillTile(
          dstTileU, SRC_PITCH_UV, static_cast<unsigned char*>(0), 0,
          tileW / 2, tileH / 2, static_cast<unsigned char>(componentLut[*(srcU + tileCtrUV)]));
        fillTile(
          dstTileV, SRC_PITCH_UV, static_cast<unsigned char*>(0), 0,
          tileW / 2, tileH / 2, static_cast<unsigned char>(componentLut[*(srcV + tileCtrUV)]));

      }

    }

  }

}



template<typename Tsample, typename Tpixel>
void TurnsTile::fillTile(
  Tsample* dstp, const int DST_PITCH,
  const Tsample* srcp, const int SRC_PITCH,
  const int width, const int height, const Tpixel fillVal) const
{

  int widthSamples = width * bytesPerPixel;

  if (srcp) {

    host->BitBlt(
      dstp, DST_PITCH, srcp, SRC_PITCH, widthSamples, height);

  } else {

    for (int h = 0; h < height; ++h) {
    
      Tsample* lineStart = dstp + (DST_PITCH * h);
      Tsample* lineEnd = lineStart + widthSamples;
      std::fill(
        reinterpret_cast<Tpixel*>(lineStart),
        reinterpret_cast<Tpixel*>(lineEnd),
        fillVal);
    
    }

  }

}



// Round 'num' to the nearest multiple of 'mod', in direction 'dir', limiting
// the result to the range 'min' through 'max'.
int TurnsTile::mod(int num, int mod, int min, int max, int dir)
{

  double base = static_cast<double>(num) / static_cast<double>(mod);

  int rounded;
  if (dir == -1)
    rounded = static_cast<int>(floor(base)) * mod;
  else if (dir == 1)
    rounded = static_cast<int>(ceil(base)) * mod;
  else
    rounded = static_cast<int>(base + 0.5) * mod;

  return rounded >= min && rounded <= max ? rounded : rounded < min ? min : max;

}



int TurnsTile::gcf(int a, int b) {  

  if (b == 0)
    return a;
  else
    return TurnsTile::gcf(b, a % b);

}