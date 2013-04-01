//
//          TurnsTile 0.1.0 for AviSynth 2.5.x
//
//  Turns video into a mosaic built from pieces of a custom tile sheet.
//  Latest release always available at http://www.gyroshot.com/turnstile.htm
//
//          Copyright 2010 Robert Martens  robert.martens@gmail.com
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



#include <math.h>

#include "windows.h"
#include "avisynth.h"

#include "TurnsTile.h"



TurnsTile::TurnsTile( PClip _child, PClip _tileSheet, int _tileW, int _tileH, int _res, int _mode, IScriptEnvironment* env) :
  GenericVideoFilter(_child), tileSheet(_tileSheet), tileW(_tileW), tileH(_tileH), mode(_mode)
{

  VideoInfo vi2 = tileSheet->GetVideoInfo();

  if ( !(vi.IsRGB() || vi.IsYUY2()) )
    env->ThrowError("TurnsTile: Only RGB and YUY2 input supported!");

  if ( !vi2.IsSameColorspace(vi) )
    env->ThrowError("TurnsTile: c and tilesheet must share a colorspace!");

  if (tileW > vi.width || tileW > vi2.width)
    env->ThrowError("TurnsTile: tilew must not exceed width of c or tilesheet!");

  if (tileH > vi.height || tileH > vi2.height)
    env->ThrowError("TurnsTile: tileh must not exceed height of c or tilesheet!");

  if (vi.width % tileW != 0 || vi2.width % tileW != 0)
    env->ThrowError("TurnsTile: Width of both c and tilesheet must be a multiple of tilew!");

  if (vi.height % tileH != 0 || vi2.height % tileH != 0)
    env->ThrowError("TurnsTile: Height of both c and tilesheet must be a multiple of tileh!");

  int minTileW = vi.IsRGB() ? 1 : 2;

  if (tileW < minTileW) {
    vi.IsRGB() ?
      env->ThrowError("TurnsTile: tilew must be at least 1 for RGB input!") :
    env->ThrowError("TurnsTile: tilew must be at least 2 for YUY2 input!");
  }

  if (tileH < 1)
    env->ThrowError("TurnsTile: tileh must be at least 1!");
  
  if ( mode == 4 && vi.IsRGB24() )
    env->ThrowError("TurnsTile: Mode 4 not available with RGB24 input!");
  
  srcCols = vi.width / tileW;
  srcRows = vi.height / tileH;

  sheetCols = vi2.width / tileW;
  sheetRows = vi2.height / tileH;
  sheetTiles = sheetCols * sheetRows;
  
  res = static_cast<double> (_res > 0 ? _res : sheetCols / 2);

  idxRangeMax = static_cast<double> (sheetTiles - 1);

  csp = vi.IsRGB32() ?  1 :
        vi.IsRGB24() ?  2 :
        vi.IsYUY2() ?   3 : 0;

  wStep = vi.IsRGB() ? 1 : 2;
  
  bytesPerPixel = vi.IsRGB32() ?  4 :
                  vi.IsRGB24() ?  3 :
                  vi.IsYUY2() ?   2 : 0;

  tileBytes = tileW * bytesPerPixel;
    
}



TurnsTile::~TurnsTile()
{

}



PVideoFrame __stdcall TurnsTile::GetFrame(int n, IScriptEnvironment* env)
{

  PVideoFrame src =             child->GetFrame(n, env);
  PVideoFrame sheetframe =      tileSheet->GetFrame(n, env);
  PVideoFrame dst =             env->NewVideoFrame(vi);
		
  const unsigned char* srcp =   src->GetReadPtr();
  const unsigned char* sheetp = sheetframe->GetReadPtr();
  unsigned char* dstp =         dst->GetWritePtr();
  
  const int SRC_PITCH =         src->GetPitch();
  const int SHEET_PITCH =       sheetframe->GetPitch();
  const int DST_PITCH =         dst->GetPitch();
  
  // Once finished drawing a tile, I need to move the sheet pointer to get
  // ready to read the next one. Instead of trying to calculate a relative
  // offset from whatever tile we've just read from the sheet, I found it
  // easiest to simply declare an origin variable here that I can return to
  // at the end of every run through the col loop.
     
  const unsigned char* const SHEET_ORIGIN = sheetp;

  for (int row = 0; row < srcRows; row++) {
  
    unsigned char* rowStart = dstp;

    for (int col = 0; col < srcCols; col++) {
      
      dstp = rowStart + (col * tileBytes);

      // For this variable, tileCtr, I made a real rookie mistake by failing to
      // clearly define the order of operations. The parentheses for tileH / 2
      // turned out to be vital when a user enters a tileh of 1; if the truncation
      // of the integer division's result isn't performed before the multiply
      // by SRC_PITCH, the output image is shifted horizontally by half of its
      // own width. I didn't encounter that problem with tileW / 2, but after the
      // other issue I decided to be safe and wrap it in parentheses too.

      const unsigned char* tileCtr =  srcp +
                                      (col * tileBytes) +             // Go to tile's first pixel,
                                      ((tileW / 2) * bytesPerPixel) + // then center horizontally,
                                      (SRC_PITCH * (tileH / 2));      // and vertically.

      double rawIdx = 0.0;

      if (csp == 3) {   // Check one boolean value, instead of hitting vi.IsYUY2()
        switch (mode) { // every loop.
          case 1 : rawIdx =     *(tileCtr);     // Y1
            break;
          case 2 : rawIdx =     *(tileCtr + 1); // U
            break;
          case 3 : rawIdx =     *(tileCtr + 2); // Y2
            break;
          case 4 : rawIdx =     *(tileCtr + 3); // V
            break;
          default : rawIdx = (  *(tileCtr) +
                                *(tileCtr + 2) ) / 2.0;
            break;
        }
      } else {
          switch (mode) {
            case 1 : rawIdx =    *(tileCtr);      // Blue
              break;
            case 2 : rawIdx =    *(tileCtr + 1);  // Green
              break;
            case 3 : rawIdx =    *(tileCtr + 2);  // Red
              break;
            case 4 : rawIdx =    *(tileCtr + 3);  // Alpha
              break;
            default : rawIdx = ( *(tileCtr) +
                                 *(tileCtr + 1) +
                                 *(tileCtr + 2) ) / 3.0;
              break;
          }
        }
      
      // Analyzing the pixels' values will always (at least for 8bpc color, if I
      // understand correctly) produce a value from 0 through 255. tileSheet may
      // have more tiles than that, or fewer, so I need to scale the value to
      // the appropriate range.

      int tileIdx = scaleToRange(rawIdx, res, idxRangeMax);
		
      // If I didn't have to worry about the differences in data storage between
      // RGB and YUV, I would skip declaring tileIdxY and just jump right to
      // cropTop, making it (tileIdx / sheetCols) * tileH. Since RGB is upside
      // down in memory, however, the desire for consistent tile numbering
      // demands that I create this new variable and then do the subtraction;
      // as a result, the top left tile is number 0 regardless of input
      // colorspace.

      int tileIdxY = tileIdx / sheetCols;
     
      if (vi.IsRGB())
        tileIdxY = sheetRows - 1 - tileIdxY;

      // Modulo here has the effect of "wrapping around" the horizontal tile
      // count for the sheet you've provided.
     
      int cropLeft =  (tileIdx % sheetCols) * tileBytes;
      int cropTop =   tileIdxY * tileH;

      sheetp += (SHEET_PITCH * cropTop);
		
      for (int h = 0; h < tileH; h++) {

        for (int w = 0; w < tileW; w += wStep) {
          
          int wBytes = w * bytesPerPixel;
          unsigned char* dstOffset = dstp + wBytes;      
          const unsigned char* sheetOffset = sheetp + cropLeft + wBytes;
          
          if (csp == 1 || csp == 3) {
            
            // It just so happens that RGB32 and YUY2 both let me work four
            // bytes at a time, wStep being the only change that's needed.
            // Casting the two offset variables as I do lets me copy four bytes
            // in one shot, as opposed to doing four separate one byte copies.
            // Running a few passes of a test render shows only a very slight
            // improvement in speed, but this required so little effort to
            // implement that I'd say it's worth it.

            *(reinterpret_cast<unsigned int*> (dstOffset)) = *(reinterpret_cast<const unsigned int*> (sheetOffset));

          } else {

            *(dstOffset) =      *(sheetOffset);
            *(dstOffset + 1) =	*(sheetOffset + 1);
            *(dstOffset + 2) =	*(sheetOffset + 2);

          }
          
        }

        sheetp += SHEET_PITCH;
        dstp += DST_PITCH;

      }

      // Finished drawing tile, reset to first position to prep for new
      // cropLeft and cropTop calculation on next run through col.
     
      sheetp = SHEET_ORIGIN;
      		
    }

    // Finally, after one row of tiles is done I knock both pointers up one
    // tile height to get set for drawing the next. srcp has stayed at the head
    // of the row this entire time, so it simply needs to be moved up. dstp, on
    // the other hand, has been shifting across the row for each tile, so I just
    // use the copy I made earlier, rowStart, and add a tile height to it.

    srcp += (SRC_PITCH * tileH);
    dstp = rowStart + (DST_PITCH * tileH);

  }

  return dst;

}



int TurnsTile::scaleToRange(double rawIdx, double res, double outMax)
{
  
  // The proper, generic form of this scaling formula would be:
  //
  // ((number) /
  //   ((inMax - inMin) / (outMax - outMin))) +
  // outMin
  //
  // Since the minimums for my purposes are always 0, and the input max is
  // always 255, I'm able to simpify the process, as seen below. Also of note
  // is that I do the math with doubles; if the output range is larger than the
  // input range, the result of that division will be between 0 and 1, which
  // would round to 0 with integers and throw a divide by zero error when then
  // used as the divisor under 'number'. There may be a better way to do all of
  // this, math was never my strong suit.
 
  double scaled = rawIdx / (255.0 / outMax);
    
  // Since I only grab the values for one pixel per tile, the results tend to
  // be slightly erratic, producing what looks like a noisy result. Averaging
  // the data for every pixel in a tile (ultimately examining every pixel in
  // the frame) would be quite a bit more work, but there's an easy way to
  // simulate the effect. Rounding each rawIdx to the nearest multiple of res
  // calms the picture, essentially introducing banding, which makes the output
  // video less objectionable.
  //
  // Also, since scaled is limited to outMax, and outMax is based on sheetTiles,
  // which is an integer, scaled should (if I'm not mistaken) never exceed the
  // possible range of values for an int. As a result, I could replace the floor
  // below with a cast to int, as such:
  //
  // int mod = static_cast<int>((scaled / res) + 0.5) * static_cast<int>(res);
  //
  // This would truncate the value, rounding toward zero, which for positive
  // numbers is the same as floor(). That would let me avoid including math.h,
  // but at my skill level, or lack thereof, I'd rather not get into tricks like
  // that right out of the gate. Not to mention that I tried it, only to find
  // no performance benefit.

  double mod = floor((scaled / res) + 0.5) * res;
  
  return static_cast<int> ( mod > outMax ? outMax : mod );

}



AVSValue __cdecl Create_TurnsTile(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new TurnsTile(  args[0].AsClip(),
                         args[1].AsClip(),
                         args[2].AsInt(16),
                         args[3].AsInt(16),
                         args[4].AsInt(0),
                         args[5].AsInt(0),
                         env);
}



extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
    env->AddFunction("TurnsTile", "cc[tileW]i[tileH]i[res]i[mode]i", Create_TurnsTile, 0);
    
    return "`TurnsTile' TurnsTile plugin";

}