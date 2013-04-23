#ifndef __CLUTer_H__
#define __CLUTer_H__



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



class CLUTer : public GenericVideoFilter
{

public:

  CLUTer(  PClip _child, PClip _palette,
           int _pltFrame, bool _interlaced,
           IScriptEnvironment* env);

  ~CLUTer();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

private:

  std::vector<unsigned char> vecRY, vecGU, vecBV;

  int bytesPerPixel, lumaW, lumaH;

  void processFrameBgr(
    const unsigned char* srcp, unsigned char* dstp,
    int width, int height,
    const int SRC_PITCH, const int DST_PITCH);

  void processFrameYuyv(
    const unsigned char* srcp, unsigned char* dstp,
    int width, int height,
    const int SRC_PITCH, const int DST_PITCH);

  void processFramePlanar(
    const unsigned char* srcY,
    const unsigned char* srcU,
    const unsigned char* srcV,
    unsigned char* dstY,
    unsigned char* dstU,
    unsigned char* dstV,
    const int SRC_WIDTH_UV, const int SRC_HEIGHT_UV,
    const int SRC_PITCH_Y, const int SRC_PITCH_UV,
    const int DST_PITCH_Y, const int DST_PITCH_UV);

  void buildPaletteBgr(
    const unsigned char* pltp, int width, int height, const int PLT_PITCH);

  void buildPaletteYuyv(
    const unsigned char* pltp, int width, int height, const int PLT_PITCH);

  void buildPalettePlanar(
    const unsigned char* pltY,
    const unsigned char* pltU,
    const unsigned char* pltV,
    int widthUV, int heightUV, const int PLT_PITCH_Y, const int PLT_PITCH_UV);

  void fillComponentVectors(std::vector<int>* pltMain);

};



#endif // __CLUTer_H__