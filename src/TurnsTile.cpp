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



TurnsTile::TurnsTile( PClip _child, PClip _tilesheet, VideoInfo _vi2,
                      int _tileW, int _tileH, int _res, int _mode,
                      const char* _levels, int _loTile, int _hiTile,
                      IScriptEnvironment* env) :
GenericVideoFilter(_child), tilesheet(_tilesheet),
tileW(_tileW), tileH(_tileH), mode(_mode),
srcRows(vi.height / tileH), srcCols(vi.width / tileW),
shtRows(_vi2.height / tileH), shtCols(_vi2.width / tileW),
bytesPerPixel(vi.BytesFromPixels(1)), tileBytes(tileW * bytesPerPixel),
host(env)
{

  if (vi.IsYV12())
    lumaW = 2, lumaH = 2;
  else if (vi.IsYUY2())
    lumaW = 2, lumaH = 1;
  else
    lumaW = 1, lumaH = 1;

  tileW_U = tileW / lumaW,
  tileH_U = tileH / lumaH;

  int idxInMin = 0;
  if (strcmp(_levels, "pc") != 0)
    idxInMin = 16;

  int idxInMax = 255;
  if (strcmp(_levels, "pc") != 0)
    if (mode > lumaW * lumaH)
      idxInMax = 240;
    else
      idxInMax = 235;

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
  depthMod = static_cast<int>(ceil(  (_hiTile - _loTile) /
                                     (pow(2.0, _res) - 1.0)  ));

  if (tilesheet) {

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

      lut.push_back(mod(scaled, depthMod, _loTile, _hiTile, 0));

    }

  } else {

    // No need to scale 'in' here, as above, since this only deals with
    // component values (not tile indices), which with 8bpc color will always
    // be less than 256.
    for (int in = 0; in < 256; ++in)
      lut.push_back(mod(in, depthMod, _loTile, _hiTile, 0));

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
    SRC_PITCH_U = src->GetPitch(PLANAR_U),
    SHT_PITCH_Y = 0,
    SHT_PITCH_U = 0,
    DST_PITCH_Y = dst->GetPitch(PLANAR_Y),
    DST_PITCH_U = dst->GetPitch(PLANAR_U);

  if (tilesheet) {

    sht = tilesheet->GetFrame(n, env);

    shtY = sht->GetReadPtr(PLANAR_Y),
    shtU = sht->GetReadPtr(PLANAR_U),
    shtV = sht->GetReadPtr(PLANAR_V);

    SHT_PITCH_Y = sht->GetPitch(PLANAR_Y),
    SHT_PITCH_U = sht->GetPitch(PLANAR_U);

  }

  if (vi.IsPlanar())
    processFramePlanar(
      srcY, srcU, srcV,
      shtY, shtU, shtV,
      dstY, dstU, dstV,
      SRC_PITCH_Y, SRC_PITCH_U,
      SHT_PITCH_Y, SHT_PITCH_U,
      DST_PITCH_Y, DST_PITCH_U);
  else
    processFramePacked(srcY, shtY, dstY, SRC_PITCH_Y, SHT_PITCH_Y, DST_PITCH_Y);

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

      unsigned char* dstTile = dstp + dstRow + curCol;

      int ctrW = mod(tileW / 2, lumaW, 0, tileW, -1) * bytesPerPixel,
          ctrH = mod(tileH / 2, lumaH, 0, tileH, -1) * SRC_PITCH;

      int tileCtr = srcRow + curCol + ctrW + ctrH;

      if (tilesheet) {

        int tileIdx;
        if (mode > 0) {

          tileIdx = lut[*(srcp + tileCtr + (mode - 1))];

        } else {

          // The hardcoded three assumes the only packed formats that might come
          // this way are RGB32, RGB24, and YUY2, which is true of Avisynth.
          int sum = 0, count = 0;
          for (int i = 0; i < 3; i += lumaW, ++count)
            sum += *(srcp + tileCtr + i);
          tileIdx = lut[sum / count];

        }

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
        int cropLeft = (tileIdx % shtCols) * tileBytes,
            cropTop = SHT_PITCH * tileIdxY * tileH;

        const unsigned char* shtTile = shtp + cropTop + cropLeft;

        fillTile(dstTile, DST_PITCH, shtTile, SHT_PITCH, tileW, tileH, 0);

      } else {

        unsigned char
          by = lut[*(srcp + tileCtr)],
          gu = lut[*(srcp + tileCtr + 1)],
          ry = lut[*(srcp + tileCtr + 2)],
          av = lut[*(srcp + tileCtr + 3)];

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
                *(dstp + dstOffset + i) = lut[*(srcp + tileCtr + i)];

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
  const int SRC_PITCH_Y, const int SRC_PITCH_U,
  const int SHT_PITCH_Y, const int SHT_PITCH_U,
  const int DST_PITCH_Y, const int DST_PITCH_U)
{

  for (int row = 0; row < srcRows; ++row) {

    int srcRowY = SRC_PITCH_Y * row * tileH,
        srcRowU = SRC_PITCH_U * row * tileH_U;

    int dstRowY = DST_PITCH_Y * row * tileH,
        dstRowU = DST_PITCH_U * row * tileH_U;

    for (int col = 0; col < srcCols; ++col) {

      int curColY = col * tileW,
          curColU = col * tileW_U;

      unsigned char
          * dstTileY = dstY + dstRowY + curColY,
          * dstTileU = dstU + dstRowU + curColU,
          * dstTileV = dstV + dstRowU + curColU;

      int ctrW_Y = mod(tileW / 2, lumaW, 0, tileW, -1),
          ctrW_U = mod(tileW_U / 2, lumaW, 0, tileW_U, -1);

      int ctrH_Y = mod(tileH / 2, lumaH, 0, tileH, -1) * SRC_PITCH_Y,
          ctrH_U = mod(tileH_U / 2, lumaH, 0, tileH_U, -1) * SRC_PITCH_U;

      int tileCtrY = srcRowY + curColY + ctrW_Y + ctrH_Y,
          tileCtrU = srcRowU + curColU + ctrW_U + ctrH_U;

      if (tilesheet) {

        int tileIdx;
        if (mode == lumaW * lumaH + 2) {

          tileIdx = lut[*(srcV + tileCtrU)];

        } else if (mode == lumaW * lumaH + 1) {

          tileIdx = lut[*(srcU + tileCtrU)];

        } else {

          if (mode > 0) {

            // This works assuming the luma samples in a macropixel are treated
            // as being numbered from zero, left to right, top to bottom.
            int lumaModeOfs = (((mode - 1) / lumaH) * SRC_PITCH_Y) +
                              ((mode - 1) % lumaW);
            tileIdx = lut[*(srcY + tileCtrY + lumaModeOfs)];

          } else {

            int sum = 0, count = 0;
            for (int i = 0; i < lumaH; ++i)
              for (int j = 0; j < lumaW; ++j, ++count)
                sum += *(srcY + tileCtrY + (SRC_PITCH_Y * i) + j);
            tileIdx = lut[sum / count];

          }

        }

        int cropLeftY = (tileIdx % shtCols) * tileW,
            cropLeftU = (tileIdx % shtCols) * tileW_U,
            cropTopY = (tileIdx / shtCols) * SHT_PITCH_Y * tileH,
            cropTopU = (tileIdx / shtCols) * SHT_PITCH_U * tileH_U;

        const unsigned char
          * shtTileY = shtY + cropLeftY + cropTopY,
          * shtTileU = shtU + cropLeftU + cropTopU,
          * shtTileV = shtV + cropLeftU + cropTopU;

        fillTile(
          dstTileY, SRC_PITCH_Y, shtTileY, SHT_PITCH_Y, tileW, tileH, 0);
        fillTile(
          dstTileU, SRC_PITCH_U, shtTileU, SHT_PITCH_U, tileW_U, tileH_U, 0);
        fillTile(
          dstTileV, SRC_PITCH_U, shtTileV, SHT_PITCH_U, tileW_U, tileH_U, 0);

      } else {

        fillTile(
          dstTileY, SRC_PITCH_Y, static_cast<unsigned char*>(0), 0,
          tileW, tileH,
          static_cast<unsigned char>(lut[*(srcY + tileCtrY)]));
        fillTile(
          dstTileU, SRC_PITCH_U, static_cast<unsigned char*>(0), 0,
          tileW_U, tileH_U,
          static_cast<unsigned char>(lut[*(srcU + tileCtrU)]));
        fillTile(
          dstTileV, SRC_PITCH_U, static_cast<unsigned char*>(0), 0,
          tileW_U, tileH_U,
          static_cast<unsigned char>(lut[*(srcV + tileCtrU)]));

      }

    }

  }

}



int TurnsTile::gcf(int a, int b) {

  if (b == 0)
    return a;
  else
    return gcf(b, a % b);

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