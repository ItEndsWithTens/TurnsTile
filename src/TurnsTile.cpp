//
//          TurnsTile 0.2.1 for AviSynth 2.5.x
//
//  Provides a mosaic effect based on either clip contents or a user-defined
//  tile sheet. Latest release hosted at http://www.gyroshot.com/turnstile.htm
//
//          Copyright 2010-2011 Robert Martens  robert.martens@gmail.com
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



// If AviSynth is ever updated to support more than eight bits per component,
// and for some reason I'm not around to update TurnsTile, changing these should
// get you at least some of the way toward proper operation.
#define MAX_BIT_DEPTH   8
#define LEVELS_PC_MIN   0
#define LEVELS_PC_MAX   255
#define LEVELS_TV_MIN   16
#define LEVELS_TV_MAXY  235
#define LEVELS_TV_MAXUV 240

// Tiles default to this value on both axes, but either will be reduced as
// necessary to arrive at a factor of the respective clip dimension. Both axes
// will also be changed to provide square tiles, even if that means going down
// to 1x1. See Create_TurnsTile() for full details.
#define DEFAULT_TILESIZE 16



TurnsTile::TurnsTile( PClip _child, PClip _tileSheet, int _tileW, int _tileH, int _res,
                      int _mode, AVSValue _levels, int _lotile, int _hitile, IScriptEnvironment* env) :
                      GenericVideoFilter(_child), tileSheet(_tileSheet), tileW(_tileW), tileH(_tileH),
                      res(_res), mode(_mode), levels(string(_levels.AsString()))
{
  
  userSheet = true;
  VideoInfo vi2 = tileSheet->GetVideoInfo();

  csp = vi.IsRGB32() ?  1 :
        vi.IsRGB24() ?  2 :
        vi.IsYUY2() ?   3 : 0;
    
  if (mode == 4 && csp == 2)
    env->ThrowError("TurnsTile: Mode 4 not available with RGB24 input!");
  
  srcCols = vi.width / tileW;
  srcRows = vi.height / tileH;

  sheetCols = vi2.width / tileW;
  sheetRows = vi2.height / tileH;
  
  idxInMin =  levels == "pc" ? LEVELS_PC_MIN : LEVELS_TV_MIN;
  idxInMax =  levels == "pc" ? LEVELS_PC_MAX :
              vi.IsYUY2() && (mode == 2 || mode == 4) ? LEVELS_TV_MAXUV: LEVELS_TV_MAXY;
  
  idxOutMin = _lotile;
  idxOutMax = _hitile;

  wStep = vi.IsRGB() ? 1 : 2;
  
  bytesPerPixel = vi.IsRGB32() ?  4 :
                  vi.IsRGB24() ?  3 :
                  vi.IsYUY2() ?   2 : 0;

  tileBytes = tileW * bytesPerPixel;

  idxScaleFactor =  static_cast<double> (idxInMax - idxInMin) /
                    static_cast<double> (idxOutMax - idxOutMin);

  // An easy way to simulate the look of decreased bit depth; treat 'res'
  // as desired number of bits per component, then cut the output range
  // into as many steps as said number of bits would provide.

  depthStep = static_cast<int>  (
                                  ceil(
                                    static_cast<double> (idxOutMax - idxOutMin) /
                                    pow(2.0,static_cast<double> (res))
                                  )
                                );

  switch (csp) {
  case 1 : copyMode = 1;
    break;
  case 2 : copyMode = 2;
    break;
  case 3 : copyMode = 1;
    break;
  default : copyMode = 0;
    break;
  }
    
}



TurnsTile::TurnsTile( PClip _child, int _tileW, int _tileH, int _res, int _mode, AVSValue _levels,
                      int _lotile, int _hitile, IScriptEnvironment* env) :
                      GenericVideoFilter(_child), tileW(_tileW), tileH(_tileH), res(_res),
                      levels(string(_levels.AsString()))
{
  
  userSheet = false;
  tileSheet = _child;
  VideoInfo vi2 = tileSheet->GetVideoInfo();
  
  csp = vi.IsRGB32() ?  1 :
        vi.IsRGB24() ?  2 :
        vi.IsYUY2() ?   3 : 0;
    
  srcCols = vi.width / tileW;
  srcRows = vi.height / tileH;

  sheetCols = vi2.width / tileW;
  sheetRows = vi2.height / tileH;

  wStep = vi.IsRGB() ? 1 : 2;
  
  bytesPerPixel = vi.IsRGB32() ?  4 :
                  vi.IsRGB24() ?  3 :
                  vi.IsYUY2() ?   2 : 0;

  tileBytes = tileW * bytesPerPixel;

  idxInMin =  levels == "pc" ? LEVELS_PC_MIN : LEVELS_TV_MIN;
  idxInMax =  levels == "pc" ? LEVELS_PC_MAX :
              vi.IsYUY2() && (mode == 2 || mode == 4) ? LEVELS_TV_MAXUV: LEVELS_TV_MAXY;
  
  idxOutMin = idxInMin;
  idxOutMax = idxInMax;
  
  idxScaleFactor = static_cast<double> (idxInMax - idxInMin) /
                   static_cast<double> (idxOutMax - idxOutMin);

  depthStep = static_cast<int>  (
                                  ceil(
                                    static_cast<double> (idxOutMax - idxOutMin) /
                                    pow(2.0,static_cast<double> (res))
                                  )
                                );

  switch (csp) {
  case 1 : copyMode = depthStep == 1 ? 1 : 4;
    break;
  case 2 : copyMode = depthStep == 1 ? 2 : 5;
    break;
  case 3 : copyMode = depthStep == 1 ? 3 : 6;
    break;
  default : copyMode = 0;
    break;
  }           
    
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

      // For this next set of variables, I made a real rookie mistake by failing to
      // clearly define the order of operations. The parentheses for tileH / 2
      // turned out to be vital when a user enters a tileh of 1; if the truncation
      // of the integer division's result isn't performed before the multiply
      // by SRC_PITCH, the output image is shifted horizontally by half of its
      // own width.
      //
      // Likewise, for centering our pointer horizontally, I need to round to the
      // next lowest multiple of tileW, hence the divide-and-multiply bit below.
      // Integer operations truncate the result, which provides the desired effect.
      // The issue didn't arise with RGB data, but with YUY2 and a minimum width
      // tile, I ended up with the pointer at Y2 instead of Y1, which meant a
      // horizontal shift in the image and swapped U and V components. It's hard
      // to explain with my lack of experience, but can be demonstrated with this:
      //
      // int ctrH = (tileW / 2) * tileW * bytesPerPixel;
      //
      // Replace the below ctrH with that, then call TurnsTile(c,2,1) where c is
      // a YUY2 clip.

      int ctrH = ((tileW / 2) / tileW) * tileW * bytesPerPixel; 
      int ctrV = SRC_PITCH * (tileH / 2);
      
      const unsigned char* tileCtr =  srcp + (col * tileBytes) + ctrH + ctrV;
      
      int cropLeft = 0, cropTop = 0;

      if (userSheet) {
        int rawIdx = 0;

        if (csp == 3) {   // Perform one logical comparison, instead of hitting
          switch (mode) { // vi.IsYUY2() every loop.
          case 1 : rawIdx =     *(tileCtr);     // Y1
            break;
          case 2 : rawIdx =     *(tileCtr + 1); // U
            break;
          case 3 : rawIdx =     *(tileCtr + 2); // Y2
            break;
          case 4 : rawIdx =     *(tileCtr + 3); // V
            break;
          default : rawIdx = (  *(tileCtr) +
                                *(tileCtr + 2) ) / 2;
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
                                 *(tileCtr + 2) ) / 3;
              break;
            }
          }

        // Analyzing the pixels' values will always (at least for 8bpc color, if I
        // understand correctly) produce a value from 0 through 255. tileSheet may
        // have more tiles than that, or fewer, so I need to scale the value to
        // the appropriate range.

        int tileIdx = scaleToRange(rawIdx, idxScaleFactor, idxOutMin, idxOutMax, depthStep);
		
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
      
        cropLeft =  (tileIdx % sheetCols) * tileBytes;
        cropTop =   tileIdxY * tileH;

        sheetp += (SHEET_PITCH * cropTop);
      }
		
      for (int h = 0; h < tileH; h++) {

        for (int w = 0; w < tileW; w += wStep) {
          
          int wBytes = w * bytesPerPixel;
          unsigned char* dstOffset = dstp + wBytes;
          const unsigned char* sheetOffset = userSheet ? sheetp + cropLeft + wBytes : tileCtr;

          switch (copyMode) {

            // You'll note that there seem to be no cases below to handle the
            // simultaneous use of a tilesheet and depthStep > 1; that's due to
            // the fact that the components themselves are untouched when using
            // an external sheet, and it's instead tileIdx that gets rounded.

          case 1 : // RGB32 with tilesheet, RGB32 without tilesheet but depthStep == 1, YUY2 with tilesheet
            *(reinterpret_cast<unsigned int*> (dstOffset)) = *(reinterpret_cast<const unsigned int*> (sheetOffset));
            break;

          case 2 : // RGB24 with tilesheet, RGB24 without tilesheet but depthStep == 1
            *(dstOffset) =      *(sheetOffset);
            *(dstOffset + 1) =	*(sheetOffset + 1);
            *(dstOffset + 2) =	*(sheetOffset + 2);
            break;

          case 3 : // YUY2 without tilesheet, depthStep == 1
            *(dstOffset) =      *(sheetOffset);
            *(dstOffset + 1) =  *(sheetOffset + 1);
            *(dstOffset + 2) =  *(sheetOffset); // Same as Y1 to prevent stripes
            *(dstOffset + 3) =  *(sheetOffset + 3);
            break;
            
          case 4 : // RGB32 without tilesheet, depthStep > 1
            *(dstOffset) =      static_cast<char>(mod(*(sheetOffset),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 1) =  static_cast<char>(mod(*(sheetOffset + 1),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 2) =  static_cast<char>(mod(*(sheetOffset + 2),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 3) =  static_cast<char>(mod(*(sheetOffset + 3),depthStep,idxOutMin,idxOutMax));
            break;

          case 5 : // RGB24 without tilesheet, depthStep > 1
            *(dstOffset) =      static_cast<char>(mod(*(sheetOffset),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 1) =  static_cast<char>(mod(*(sheetOffset + 1),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 2) =  static_cast<char>(mod(*(sheetOffset + 2),depthStep,idxOutMin,idxOutMax));
            break;

          case 6 : // YUY2 without tilesheet, depthStep > 1
            *(dstOffset) =      static_cast<char>(mod(*(sheetOffset),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 1) =  static_cast<char>(mod(*(sheetOffset + 1),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 2) =  static_cast<char>(mod(*(sheetOffset),depthStep,idxOutMin,idxOutMax));
            *(dstOffset + 3) =  static_cast<char>(mod(*(sheetOffset + 3),depthStep,idxOutMin,idxOutMax));
            break;

          default :
            env->ThrowError("TurnsTile: Unsupported colorspace! My code should not have gotten this far...");
            break;
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



int TurnsTile::scaleToRange(int rawIdx, double scaleFactor, int outMin, int outMax, int res)
{

  // The proper, generic form of this scaling formula would be:
  //
  // ((number) /
  //   ((inMax - inMin) / (outMax - outMin))) +
  // outMin
  //
  // Dividing the input range by the output range gives a value I've named
  // 'scaleFactor', which for my purposes can be precomputed. I do that in the
  // constructor, so as not to waste time generating it for every component of
  // every pixel in every frame.
  //
  // rawIdx is cast to a double since the result of its division may be less
  // than 1 (maximum input range for 8bpc clips is 0-255, but you can very
  // easily have more than 255 tiles). Add 0.5, cast to int to truncate the
  // result, and I've successfully rounded the number. I insisted in version
  // 0.1.0 on using the floor() function of math.h to do the same thing, but
  // I've since come around to be comfortable doing it this way.
  
  int scaled =  static_cast<int> ((static_cast<double> (rawIdx) / scaleFactor) + 0.5) + outMin;
  
  return mod(scaled,res,outMin,outMax);

}



int TurnsTile::mod(int num, int mod, int min, int max)
{

  int base =  static_cast<int> (
                (static_cast<double> (num) /
                 static_cast<double> (mod)) + 0.5
              ) * mod;

  return base >= max ? max : base <= min ? min : base;

}



int TurnsTile_gcf(int a, int b) {

  int gcf = a < b ? a : b;
  int c = a % b;
  
  while (c > 0) {

      a = b;
      b = c;
      c = a % b;

      gcf = b;

  }

  return gcf;

}



AVSValue __cdecl Create_TurnsTile(AVSValue args, void* user_data, IScriptEnvironment* env)
{

  if (args[0].AsClip()->GetVideoInfo().IsRGB() == false &&
      args[0].AsClip()->GetVideoInfo().IsYUY2() == false)
    env->ThrowError("TurnsTile: Only RGB and YUY2 input supported!");

  int minTileW = args[0].AsClip()->GetVideoInfo().IsRGB() ? 1 : 2;

  if (args[1].IsClip()) {

    if ( !args[0].AsClip()->GetVideoInfo().IsSameColorspace(args[1].AsClip()->GetVideoInfo()) )
      env->ThrowError("TurnsTile: c and tilesheet must share a colorspace!");

    // As with most ideas I have, this factor stuff was originally much more
    // complicated. With a few days to reflect on it, though, I saw that just
    // grabbing the greatest common factor for the width of both clips, and
    // again for the heights, then figuring out the gcf of those two values
    // gives me the largest possible tile size that will allow square tiles.
    // After that, DEFAULT_TILESIZE caps the value to something sensible, and
    // one quick for loop ensures I still have a factor of the clip dimensions.

    int clipW =   args[0].AsClip()->GetVideoInfo().width,
        sheetW =  args[1].AsClip()->GetVideoInfo().width,
        gcfW =    TurnsTile_gcf(clipW,sheetW);

    int clipH =   args[0].AsClip()->GetVideoInfo().height,
        sheetH =  args[1].AsClip()->GetVideoInfo().height,
        gcfH =    TurnsTile_gcf(clipH,sheetH);
    
    int gcfBoth = TurnsTile_gcf(gcfW,gcfH),
        gcfMaster = DEFAULT_TILESIZE;

    for (gcfMaster; gcfBoth % gcfMaster > 0; gcfMaster--) {
    }

    int tileW = args[2].AsInt(gcfMaster <= minTileW ? minTileW : gcfMaster),
        tileH = args[3].AsInt(gcfMaster <= 1 ? 1 : gcfMaster);

    if (tileW < minTileW) {
      args[0].AsClip()->GetVideoInfo().IsRGB() ?
        env->ThrowError("TurnsTile: tilew must be at least 1 for RGB input!") :
      env->ThrowError("TurnsTile: tilew must be at least 2 for YUY2 input!");
    }
  
    if (tileH < 1)
      env->ThrowError("TurnsTile: tileh must be at least 1!");

    if (tileW > clipW || tileW > sheetW)
      env->ThrowError("TurnsTile: tilew must not exceed width of c or tilesheet!");

    if (tileH > clipH || tileH > sheetH)
      env->ThrowError("TurnsTile: tileh must not exceed height of c or tilesheet!");
        
    if (clipW % tileW > 0 || sheetW % tileW > 0)
      env->ThrowError("TurnsTile: tilew must be a factor of both c and tilesheet widths!");

    if (clipH % tileH > 0 || sheetH % tileH > 0)
      env->ThrowError("TurnsTile: tileh must be a factor of both c and tilesheet heights!");

    int sheetCols = args[1].AsClip()->GetVideoInfo().width / tileW,
        sheetRows = args[1].AsClip()->GetVideoInfo().height / tileH,
        sheetTiles = sheetCols * sheetRows;

    int tileIdxMax = sheetTiles - 1;

    int lotile =  args[7].AsInt(0) <= 0 ? 0 :
                  args[7].AsInt() >= tileIdxMax ? tileIdxMax : args[7].AsInt();

    int hitile =  args[8].AsInt(tileIdxMax) >= tileIdxMax ? tileIdxMax :
                  args[8].AsInt() <= 0 ? 0 : args[8].AsInt();

    if (lotile > hitile)
      env->ThrowError("TurnsTile: lotile cannot be greater than hitile!");

    return new TurnsTile(  args[0].AsClip(),
                           args[1].AsClip(),
                           tileW,
                           tileH,
                           args[4].AsInt(MAX_BIT_DEPTH),
                           args[5].AsInt(0),
                           env->Invoke("LCase",args[6].AsString("pc")),
                           lotile,
                           hitile,
                           env);
  } else {

    int clipW = args[0].AsClip()->GetVideoInfo().width,
        clipH = args[0].AsClip()->GetVideoInfo().height,
        gcf = TurnsTile_gcf(clipW,clipH),
        gcfMaster = DEFAULT_TILESIZE;

    for (gcfMaster; gcf % gcfMaster > 0; gcfMaster--) {
    }

    int tileW = args[1].AsInt(gcfMaster <= minTileW ? minTileW : gcfMaster),
        tileH = args[2].AsInt(gcfMaster <= 1 ? 1 : gcfMaster);
    
    if (tileW < minTileW) {
      args[0].AsClip()->GetVideoInfo().IsRGB() ?
        env->ThrowError("TurnsTile: tilew must be at least 1 for RGB input!") :
      env->ThrowError("TurnsTile: tilew must be at least 2 for YUY2 input!");
    }

    if (tileH < 1)
      env->ThrowError("TurnsTile: tileh must be at least 1!");

    if (tileW > clipW)
      env->ThrowError("TurnsTile: tilew must not exceed width of c!");

    if (tileH > clipH)
      env->ThrowError("TurnsTile: tileh must not exceed height of c!");
        
    if (clipW % tileW > 0)
      env->ThrowError("TurnsTile: tilew must be a factor of c width!");

    if (clipH % tileH > 0)
      env->ThrowError("TurnsTile: tileh must be a factor of c height!");
    
    return new TurnsTile(  args[0].AsClip(),
                           tileW,
                           tileH,
                           args[3].AsInt(MAX_BIT_DEPTH),
                           args[4].AsInt(0),
                           env->Invoke("LCase",args[5].AsString("pc")),
                           args[6].AsInt(0), // Only here to prevent
                           args[7].AsInt(0), // annoying error messages.
                           env);
  }

}



extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
    
    env->AddFunction("TurnsTile", "cc[tileW]i[tileH]i[res]i[mode]i[levels]s[lotile]i[hitile]i", Create_TurnsTile, 0);
    env->AddFunction("TurnsTile", "c[tileW]i[tileH]i[res]i[mode]i[levels]s[lotile]i[hitile]i", Create_TurnsTile, 0);
    
    return "`TurnsTile' TurnsTile plugin";

}