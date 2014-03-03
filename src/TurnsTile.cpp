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

TurnsTile::TurnsTile( PClip _child, PClip _tilesheet, VideoInfo _vi2,
                      int _tileW, int _tileH, int _res, int _mode,
                      const char* _levels, int _loTile, int _hiTile,
                      IScriptEnvironment* env) :
GenericVideoFilter(_child), tilesheet(_tilesheet),
tileW(_tileW), tileH(_tileH), mode(_mode),
srcCols(vi.width / tileW), srcRows(vi.height / tileH),
shtCols(_vi2.width / tileW), shtRows(_vi2.height / tileH),
bytesPerSample(1), spp(vi.BytesFromPixels(1) / bytesPerSample),
PLANAR(vi.IsPlanar()), YUYV(vi.IsYUY2()), BGRA(vi.IsRGB32()), BGR(vi.IsRGB24())
{

#ifdef TURNSTILE_HOST_AVISYNTH_26

  if (vi.IsYUV() && !vi.IsY8()) {
    lumaW = 1 << vi.GetPlaneWidthSubsampling(PLANAR_U);
    lumaH = 1 << vi.GetPlaneHeightSubsampling(PLANAR_U);
  } else {
    lumaW = 1;
    lumaH = 1;
  }

  // A zero height tells fillTile to skip doing any work, which speeds up Y8.
  if (vi.IsY8()) {
    tileW_U = 0;
    tileH_U = 0;
  } else {
    tileW_U = tileW / lumaW;
    tileH_U = tileH / lumaH;
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

  tileW_U = tileW / lumaW;
  tileH_U = tileH / lumaH;

#endif

  tileCtrW_Y = mod(tileW / 2, lumaW, 0, tileW, -1);
  tileCtrW_U = tileW_U / 2;
  tileCtrH_Y = mod(tileH / 2, lumaH, 0, tileH, -1);
  tileCtrH_U = tileH_U / 2;

  int idxInMin = 0;
  if (strcmp(_levels, "tv") == 0)
    idxInMin = 16;

  int idxInMax = 255;
  if (strcmp(_levels, "tv") == 0) {
    if (mode > lumaW * lumaH)
      idxInMax = 240;
    else
      idxInMax = 235;
  }

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
  int depthMod = static_cast<int>(ceil( (_hiTile - _loTile) /
                                        (pow(2.0, _res) - 1.0) ));

  if (tilesheet) {

    double factor = static_cast<double>(_hiTile - _loTile) /
                    static_cast<double>(idxInMax - idxInMin);

    for (int in = 0; in < 256; ++in) {

      // The proper, generic form of this scaling formula would be:
      //
      // (outMax - outMin) * (number - inMin)
      // ------------------------------------ + outMin
      //             inMax - inMin
      //
      // Using mod to round the result is a unique requirement of the 'res'
      // feature I've got in TurnsTile, you don't need it if only scaling.
      int scaled = static_cast<int>((in - idxInMin) * factor + 0.5) + _loTile;

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
    SRC_PITCH_SAMPLES_Y = src->GetPitch(PLANAR_Y),
    SRC_PITCH_SAMPLES_U = src->GetPitch(PLANAR_U),
    SHT_PITCH_SAMPLES_Y = 0,
    SHT_PITCH_SAMPLES_U = 0,
    DST_PITCH_SAMPLES_Y = dst->GetPitch(PLANAR_Y),
    DST_PITCH_SAMPLES_U = dst->GetPitch(PLANAR_U);

  if (tilesheet) {

    sht = tilesheet->GetFrame(n, env);

    shtY = sht->GetReadPtr(PLANAR_Y);
    shtU = sht->GetReadPtr(PLANAR_U);
    shtV = sht->GetReadPtr(PLANAR_V);

    SHT_PITCH_SAMPLES_Y = sht->GetPitch(PLANAR_Y);
    SHT_PITCH_SAMPLES_U = sht->GetPitch(PLANAR_U);

  }

  if (PLANAR)
    processFramePlanar(
      srcY, srcU, srcV,
      shtY, shtU, shtV,
      dstY, dstU, dstV,
      SRC_PITCH_SAMPLES_Y, SRC_PITCH_SAMPLES_U,
      SHT_PITCH_SAMPLES_Y, SHT_PITCH_SAMPLES_U,
      DST_PITCH_SAMPLES_Y, DST_PITCH_SAMPLES_U,
      env);
  else
    processFramePacked(
      srcY, shtY, dstY,
      SRC_PITCH_SAMPLES_Y, SHT_PITCH_SAMPLES_Y, DST_PITCH_SAMPLES_Y,
      env);

  return dst;

}



void TurnsTile::processFramePacked(
  const unsigned char* srcp,
  const unsigned char* shtp,
  unsigned char* dstp,
  const int SRC_PITCH_SAMPLES,
  const int SHT_PITCH_SAMPLES,
  const int DST_PITCH_SAMPLES,
  IScriptEnvironment* env)
{

  for (int row = 0; row < srcRows; ++row) {

    int srcRow = SRC_PITCH_SAMPLES * row * tileH,
        dstRow = DST_PITCH_SAMPLES * row * tileH;

    for (int col = 0; col < srcCols; ++col) {

      int curCol = col * tileW * spp;

      unsigned char* dstTile = dstp + dstRow + curCol;

      int tileCtr = srcRow + curCol +
                    (tileCtrW_Y * spp) + (tileCtrH_Y * SRC_PITCH_SAMPLES);

      if (tilesheet) {

        int tileIdx;
        if (mode > 0) {

          tileIdx = lut[*(srcp + tileCtr + (mode - 1))];

        } else {

          // The hardcoded three assumes the only packed formats that might come
          // this way are RGB32, RGB24, and YUY2, which is true of Avisynth.
          int sum = 0, count = 0;
          for (int i = 0; i < 3; i += lumaW) {
            sum += *(srcp + tileCtr + i);
            ++count;
          }
          tileIdx = lut[sum / count];

        }

        // If I didn't have to worry about the difference in layout between RGB
        // and YUV, I would skip declaring tileIdxY and just jump right to
        // cropTop, making it SHT_PITCH_SAMPLES * (tileIdx / shtCols) * tileH.
        // Since RGB is "upside down" in memory, however, the desire for
        // consistent tile numbering demands that I create this new variable and
        // then do the subtraction; as a result, the top left tile is number 0
        // regardless of input colorspace.
        int tileIdxY = tileIdx / shtCols;

        if (BGRA || BGR)
          tileIdxY = shtRows - 1 - tileIdxY;

        // Modulo here has the effect of "wrapping around" the horizontal tile
        // count for the sheet you've provided.
        int cropLeft = (tileIdx % shtCols) * tileW * spp,
            cropTop = SHT_PITCH_SAMPLES * tileIdxY * tileH;

        const unsigned char* shtTile = shtp + cropTop + cropLeft;

        fillTile(
          dstTile, DST_PITCH_SAMPLES,
          shtTile, SHT_PITCH_SAMPLES,
          tileW, tileH, 0, env);

      } else {

        if (BGRA || YUYV) {

          unsigned char
            by = lut[*(srcp + tileCtr)],
            gu = lut[*(srcp + tileCtr + 1)],
            ry = lut[*(srcp + tileCtr + 2)],
            av = lut[*(srcp + tileCtr + 3)];

          unsigned int fillVal;
          if (BGRA)
            fillVal = (av << 24) | (ry << 16) | (gu << 8) | by;
          else
            fillVal = (av << 24) | (by << 16) | (gu << 8) | by;

          fillTile(
            dstTile, DST_PITCH_SAMPLES, static_cast<const unsigned char*>(0), 0,
            tileW, tileH, fillVal, env);

        } else {

          // For the time being, I'm giving RGB24 its own slow, manual loop,
          // instead of trying to get fillTile to handle a funny stepping
          // sequence for a three byte pixel written four bytes at a time.
          unsigned char
            b = lut[*(srcp + tileCtr + 0)],
            g = lut[*(srcp + tileCtr + 1)],
            r = lut[*(srcp + tileCtr + 2)];

          for (int h = 0; h < tileH; ++h) {

            int dstLine = DST_PITCH_SAMPLES * h;

            for (int w = 0; w < tileW; ++w) {

              int dstOfs = dstRow + curCol + dstLine + (w * spp);

              *(dstp + dstOfs + 0) = b;
              *(dstp + dstOfs + 1) = g;
              *(dstp + dstOfs + 2) = r;

            }

          }

        }

      }

    }

  }

}



void TurnsTile::processFramePlanar(
  const unsigned char* srcY,
  const unsigned char* srcU,
  const unsigned char* srcV,
  const unsigned char* shtY,
  const unsigned char* shtU,
  const unsigned char* shtV,
  unsigned char* dstY,
  unsigned char* dstU,
  unsigned char* dstV,
  const int SRC_PITCH_SAMPLES_Y, const int SRC_PITCH_SAMPLES_U,
  const int SHT_PITCH_SAMPLES_Y, const int SHT_PITCH_SAMPLES_U,
  const int DST_PITCH_SAMPLES_Y, const int DST_PITCH_SAMPLES_U,
  IScriptEnvironment* env)
{

  for (int row = 0; row < srcRows; ++row) {

    int srcRowY = SRC_PITCH_SAMPLES_Y * row * tileH,
        srcRowU = SRC_PITCH_SAMPLES_U * row * tileH_U;

    int dstRowY = DST_PITCH_SAMPLES_Y * row * tileH,
        dstRowU = DST_PITCH_SAMPLES_U * row * tileH_U;

    for (int col = 0; col < srcCols; ++col) {

      int curColY = col * tileW,
          curColU = col * tileW_U;

      unsigned char
          * dstTileY = dstY + dstRowY + curColY,
          * dstTileU = dstU + dstRowU + curColU,
          * dstTileV = dstV + dstRowU + curColU;

      int
        tileCtrY = srcRowY + curColY +
                   (tileCtrW_Y * spp) + (tileCtrH_Y * SRC_PITCH_SAMPLES_Y),
        tileCtrU = srcRowU + curColU +
                   (tileCtrW_U * spp) + (tileCtrH_U * SRC_PITCH_SAMPLES_U);

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
            int lumaModeOfs = ((mode % lumaH) * SRC_PITCH_SAMPLES_Y) +
                              ((mode - 1) % lumaW);
            tileIdx = lut[*(srcY + tileCtrY + lumaModeOfs)];

          } else {

            int sum = 0, count = 0;
            for (int i = 0; i < lumaH; ++i)
              for (int j = 0; j < lumaW; ++j) {
                sum += *(srcY + tileCtrY + (SRC_PITCH_SAMPLES_Y * i) + j);
                ++count;
              }
            tileIdx = lut[sum / count];

          }

        }

        int cropLeftY = (tileIdx % shtCols) * tileW,
            cropLeftU = (tileIdx % shtCols) * tileW_U,
            cropTopY = (tileIdx / shtCols) * SHT_PITCH_SAMPLES_Y * tileH,
            cropTopU = (tileIdx / shtCols) * SHT_PITCH_SAMPLES_U * tileH_U;

        const unsigned char
          * shtTileY = shtY + cropLeftY + cropTopY,
          * shtTileU = shtU + cropLeftU + cropTopU,
          * shtTileV = shtV + cropLeftU + cropTopU;

        fillTile(
          dstTileY, DST_PITCH_SAMPLES_Y,
          shtTileY, SHT_PITCH_SAMPLES_Y,
          tileW, tileH, 0, env);
        fillTile(
          dstTileU, DST_PITCH_SAMPLES_U,
          shtTileU, SHT_PITCH_SAMPLES_U,
          tileW_U, tileH_U, 0, env);
        fillTile(
          dstTileV, DST_PITCH_SAMPLES_U,
          shtTileV, SHT_PITCH_SAMPLES_U,
          tileW_U, tileH_U, 0, env);

      } else {

        fillTile(
          dstTileY, DST_PITCH_SAMPLES_Y, static_cast<unsigned char*>(0), 0,
          tileW, tileH,
          static_cast<unsigned char>(lut[*(srcY + tileCtrY)]), env);
        fillTile(
          dstTileU, DST_PITCH_SAMPLES_U, static_cast<unsigned char*>(0), 0,
          tileW_U, tileH_U,
          static_cast<unsigned char>(lut[*(srcU + tileCtrU)]), env);
        fillTile(
          dstTileV, DST_PITCH_SAMPLES_U, static_cast<unsigned char*>(0), 0,
          tileW_U, tileH_U,
          static_cast<unsigned char>(lut[*(srcV + tileCtrU)]), env);

      }

    }

  }

}



int TurnsTile::gcf(int a, int b)
{

  if (b == 0)
    return a;
  else
    return gcf(b, a % b);

}



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

  if (rounded >= min && rounded <= max)
    return rounded;
  else if (rounded < min)
    return min;
  else
    return max;

}



template<typename Tsample, typename Tpixel>
void TurnsTile::fillTile(
  Tsample* dstp, const int DST_PITCH_SAMPLES,
  const Tsample* srcp, const int SRC_PITCH_SAMPLES,
  const int width, const int height, const Tpixel fillVal,
  IScriptEnvironment* env) const
{

  int widthSamples = width * spp;

  if (srcp) {

    int widthBytes = widthSamples * bytesPerSample;
    env->BitBlt(
      dstp, DST_PITCH_SAMPLES, srcp, SRC_PITCH_SAMPLES, widthBytes, height);

  } else {

    for (int h = 0; h < height; ++h) {

      Tsample* lineStart = dstp + (DST_PITCH_SAMPLES * h);
      Tsample* lineEnd = lineStart + widthSamples;
      std::fill(
        reinterpret_cast<Tpixel*>(lineStart),
        reinterpret_cast<Tpixel*>(lineEnd),
        fillVal);

    }

  }

}