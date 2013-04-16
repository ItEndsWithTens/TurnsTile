#ifndef __TurnsTile_H__
#define __TurnsTile_H__



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



#include <Windows.h>

#include <vector>

#include "avisynth.h"



class TurnsTile : public GenericVideoFilter
{

public:

  TurnsTile(  PClip _child, PClip _tilesheet, VideoInfo _vi2,
              int _tileW, int _tileH, int _res, int _mode,
              const char* _levels, int _loTile, int _hiTile,
              IScriptEnvironment* env);

  ~TurnsTile();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  void __stdcall processFramePacked(
    const unsigned char* srcp, const unsigned char* shtp, unsigned char* dstp,
    const int SRC_PITCH, const int SHT_PITCH, const int DST_PITCH);

  void __stdcall processFramePlanar(
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
    const int DST_PITCH_Y, const int DST_PITCH_U);

  static int gcf(int a, int b);

  static int mod(int num, int mod, int min, int max, int dir);

private:

  PClip tilesheet;

  int tileW, tileH, mode,
      srcCols, srcRows,
      shtCols, shtRows,
      bytesPerSample, samplesPerPixel,
      depthMod, lumaW, lumaH, tileW_U, tileH_U;

  IScriptEnvironment* host;

  std::vector<int> lut;

  template<typename Tsample, typename Tpixel>
  void fillTile(
    Tsample* dstp, const int DST_PITCH,
    const Tsample* srcp, const int SRC_PITCH,
    const int width, const int height, const Tpixel fillVal) const;

};

#endif // __TurnsTile_H__