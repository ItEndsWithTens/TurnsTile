//
//          TurnsTile 0.3.0 for AviSynth 2.5.x
//
//  Provides customizable mosaic and palette effects. Latest release
//  hosted at http://www.gyroshot.com/turnstile.htm
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



#include "TurnsTile.h"

#include <cmath>
#include <cstring>
#include <Windows.h>

#include "avisynth.h"



TurnsTile::TurnsTile( PClip _child, PClip _tileSheet,
                      int _tileW, int _tileH,
                      int _res, int _mode,
                      const char* _levels,
                      int _loTile, int _hiTile,
                      bool _interlaced,
                      IScriptEnvironment* env) :
                      GenericVideoFilter(
                        _interlaced && !_child->GetVideoInfo().IsFieldBased() ?
                          env->Invoke("SeparateFields", _child).AsClip() :
                        _child),
                      tileSheet(
                        _interlaced && !_tileSheet->GetVideoInfo().IsFieldBased() ?
                          env->Invoke("SeparateFields", _tileSheet).AsClip() :
                        _tileSheet),
                      tileW(_tileW), tileH(_tileH),
                      mode(_mode)
{
 
  userSheet = true;

  VideoInfo vi2 = tileSheet->GetVideoInfo();

  srcCols = vi.width / tileW;
  srcRows = vi.height / tileH;

  shtCols = vi2.width / tileW;
  shtRows = vi2.height / tileH;
  
  int idxInMin =  strcmp(_levels, "pc") == 0 ? 0 : 16;
  int idxInMax =  strcmp(_levels, "pc") == 0 ? 255 :
                  (vi.IsYUY2() && (mode == 2 || mode == 4)) ||
                  (vi.IsYV12() && (mode == 5 || mode == 6)) ? 240 : 235;

  wStep = vi.IsRGB() ? 1 : 2;
  
  bytesPerPixel = vi.IsRGB32() ?  4 :
                  vi.IsRGB24() ?  3 :
                  vi.IsYUY2() ?   2 : 0;

  tileBytes = tileW * bytesPerPixel;

  double idxScaleFactor = static_cast<double> (idxInMax - idxInMin) /
                          static_cast<double> (_hiTile - _loTile);

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
  depthStep = static_cast<int>  (
                ceil(
                  static_cast<double> (_hiTile - _loTile) /
                  (pow(2.0, static_cast<double> (_res)) - 1.0)
                )
              );

  for (int in = 0; in < 256; ++in) {

    // The proper, generic form of this scaling formula would be:
    //
    // ((number) /
    //   ((inMax - inMin) / (outMax - outMin))) +
    // outMin
    //
    // Using TurnsTile_mod() to round the result is a unique requirement of the
    // 'res' feature I've got in TurnsTile, you don't need it if only scaling.
    int scaled =  static_cast<int> (
                    (static_cast<double> (in) / idxScaleFactor) + 0.5
                  ) + _loTile;
  
    int out = TurnsTile_mod(scaled, depthStep, _loTile, _hiTile);

    tileIdxLut.push_back(out);

  }  

  copyMode = vi.IsRGB24() ? 2 : 1;

}



TurnsTile::TurnsTile( PClip _child,
                      int _tileW, int _tileH,
                      int _res,
                      int _loTile, int _hiTile,
                      bool _interlaced,
                      IScriptEnvironment* env) :
                      GenericVideoFilter(
                        _interlaced && !_child->GetVideoInfo().IsFieldBased() ?
                          env->Invoke("SeparateFields", _child).AsClip() :
                        _child),
                      tileW(_tileW), tileH(_tileH)
{
  
  userSheet = false;

  tileSheet = child;
    
  srcCols = vi.width / tileW;
  srcRows = vi.height / tileH;

  wStep = vi.IsRGB() ? 1 : 2;
  
  bytesPerPixel = vi.IsRGB32() ?  4 :
                  vi.IsRGB24() ?  3 :
                  vi.IsYUY2() ?   2 : 0;

  tileBytes = tileW * bytesPerPixel;
  
  depthStep = static_cast<int>  (
                ceil(
                  static_cast<double> (_hiTile - _loTile) /
                  (pow(2.0, static_cast<double> (_res)) - 1.0)
                )
              );

  // No need to scale 'in' here, as in the above constructor, since this
  // version of TurnsTile only deals with component values (not tile indices),
  // which with 8bpc color will always be less than 256.
  for (int in = 0; in < 256; ++in) {

    unsigned char out = static_cast<unsigned char> (
                          TurnsTile_mod(in, depthStep, _loTile, _hiTile)
                        );

    componentLut.push_back(out);

  }

  copyMode =  vi.IsRGB32() ?  3 :
              vi.IsRGB24() ?  4 :
              vi.IsYUY2() ?   5 :
              vi.IsYV12() ?   2 :
                              0;

}



TurnsTile::~TurnsTile()
{
}



PVideoFrame __stdcall TurnsTile::GetFrame(int n, IScriptEnvironment* env)
{
  return vi.IsPlanar() ?  GetFramePlanar(n, env) :
                          GetFrameInterleaved(n, env);
}



PVideoFrame __stdcall TurnsTile::GetFrameInterleaved(int n, IScriptEnvironment* env)
{

  PVideoFrame
    src = child->GetFrame(n, env),
    sht = tileSheet->GetFrame(n, env),
    dst = env->NewVideoFrame(vi);

  const unsigned char
    * srcp = src->GetReadPtr(),
    * shtp = sht->GetReadPtr();

  unsigned char
    * dstp = dst->GetWritePtr();

  const int
    SRC_PITCH = src->GetPitch(),
    SHT_PITCH = sht->GetPitch(),
    DST_PITCH = dst->GetPitch();

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

      if (userSheet) {

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
		
      for (int h = 0; h < tileH; ++h) {

        int dstLine = DST_PITCH * h,
            shtLine = SHT_PITCH * h;

        for (int w = 0; w < tileW; w += wStep) {
          
          int wBytes = w * bytesPerPixel;

          int dstOffset = dstRow + curCol + dstLine + wBytes,
              shtOffset = cropLeft + cropTop + shtLine + wBytes;

          switch (copyMode) {

          case 1 :
            // RGB32 / YUY2 with tilesheet
            *(dstp + dstOffset) =      *(shtp + shtOffset);
            *(dstp + dstOffset + 1) =  *(shtp + shtOffset + 1);
            *(dstp + dstOffset + 2) =  *(shtp + shtOffset + 2);
            *(dstp + dstOffset + 3) =  *(shtp + shtOffset + 3);
            break;

          case 2 :
            // RGB24 with tilesheet
            *(dstp + dstOffset) =     *(shtp + shtOffset);
            *(dstp + dstOffset + 1) = *(shtp + shtOffset + 1);
            *(dstp + dstOffset + 2) =	*(shtp + shtOffset + 2);
            break;

          case 3 :
            // RGB32 without tilesheet
            *(dstp + dstOffset) =      componentLut[*(shtp + tileCtr)];
            *(dstp + dstOffset + 1) =  componentLut[*(shtp + tileCtr + 1)];
            *(dstp + dstOffset + 2) =  componentLut[*(shtp + tileCtr + 2)];
            *(dstp + dstOffset + 3) =  componentLut[*(shtp + tileCtr + 3)];
            break;

          case 4 :
            // RGB24 without tilesheet
            *(dstp + dstOffset) =      componentLut[*(shtp + tileCtr)];
            *(dstp + dstOffset + 1) =  componentLut[*(shtp + tileCtr + 1)];
            *(dstp + dstOffset + 2) =  componentLut[*(shtp + tileCtr + 2)];
            break;

          case 5 :
            // YUY2 without tilesheet
            *(dstp + dstOffset) =      componentLut[*(shtp + tileCtr)];
            *(dstp + dstOffset + 1) =  componentLut[*(shtp + tileCtr + 1)];
            *(dstp + dstOffset + 2) =  componentLut[*(shtp + tileCtr)];
            *(dstp + dstOffset + 3) =  componentLut[*(shtp + tileCtr + 3)];
            break;

          default :
            env->ThrowError("TurnsTile: Invalid interleaved copyMode; "
                            "please contact me!");
            break;

          }

        }

      }
		
    }

  }

  return dst;

}



PVideoFrame __stdcall TurnsTile::GetFramePlanar(int n, IScriptEnvironment* env)
{

  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame sht = tileSheet->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  
  const unsigned char
    * srcY = src->GetReadPtr(PLANAR_Y),
    * srcU = src->GetReadPtr(PLANAR_U),
    * srcV = src->GetReadPtr(PLANAR_V);

  const unsigned char
    * shtY = sht->GetReadPtr(PLANAR_Y),
    * shtU = sht->GetReadPtr(PLANAR_U),
    * shtV = sht->GetReadPtr(PLANAR_V);

  unsigned char
    * dstY = dst->GetWritePtr(PLANAR_Y),
    * dstU = dst->GetWritePtr(PLANAR_U),
    * dstV = dst->GetWritePtr(PLANAR_V);

  const int
    SRC_PITCH_Y = src->GetPitch(PLANAR_Y),
    SHT_PITCH_Y = sht->GetPitch(PLANAR_Y),
    DST_PITCH_Y = dst->GetPitch(PLANAR_Y);

  const int
    SRC_PITCH_UV = src->GetPitch(PLANAR_U),
    SHT_PITCH_UV = sht->GetPitch(PLANAR_U),
    DST_PITCH_UV = dst->GetPitch(PLANAR_U);  

  for (int row = 0; row < srcRows; ++row) {

    int srcRowY =   SRC_PITCH_Y * row * tileH,
        srcRowUV =  SRC_PITCH_UV * row * (tileH / 2);

    int dstRowY =   DST_PITCH_Y * row * tileH,
        dstRowUV =  DST_PITCH_UV * row * (tileH / 2);

    for (int col = 0; col < srcCols; ++col) {

      int curColY =   col * tileW,
          curColUV =  col * (tileW / 2);

      // As you'll note, in the if (userSheet) code below I read four Y
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

      int cropLeftY = 0,
          cropLeftUV = 0,
          cropTopY = 0,
          cropTopUV = 0;

      if (userSheet) {

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
		    
        cropLeftY =  (tileIdx % shtCols) * tileW;
        cropLeftUV = (tileIdx % shtCols) * (tileW / 2);

        cropTopY =  SHT_PITCH_Y * (tileIdx / shtCols) * tileH;
        cropTopUV = SHT_PITCH_UV * (tileIdx / shtCols) * (tileH / 2);

      }
        
      for (int h = 0; h < tileH / 2; ++h) {

        int dstLineY =  DST_PITCH_Y * h * 2,
            dstLineUV = DST_PITCH_UV * h;

        int shtLineY =  SHT_PITCH_Y * h * 2,
            shtLineUV = SHT_PITCH_UV * h;

        for (int w = 0; w < tileW / 2; ++w) {

          int curSampleY =  w * 2,
              curSampleUV = w;

          int dstOffsetY =  dstRowY + curColY + dstLineY + curSampleY,
              dstOffsetUV = dstRowUV + curColUV + dstLineUV + curSampleUV;

          int shtOffsetY =  cropLeftY + cropTopY + shtLineY + curSampleY,
              shtOffsetUV = cropLeftUV + cropTopUV + shtLineUV + curSampleUV;

          unsigned char finalY;

          switch (copyMode) {

          case 1 : // YV12 with tilesheet
            *(dstU + dstOffsetUV) =  *(shtU + shtOffsetUV);
            *(dstV + dstOffsetUV) =  *(shtV + shtOffsetUV);

            *(dstY + dstOffsetY) =                    *(shtY + shtOffsetY);
            *(dstY + dstOffsetY + 1) =                *(shtY + shtOffsetY + 1);
            *(dstY + dstOffsetY + DST_PITCH_Y) =      *(shtY + shtOffsetY + SHT_PITCH_Y);
            *(dstY + dstOffsetY + DST_PITCH_Y + 1) =  *(shtY + shtOffsetY + SHT_PITCH_Y + 1);
            break;

          case 2 : // YV12 without tilesheet
            *(dstU + dstOffsetUV) = componentLut[*(srcU + tileCtrUV)];
            *(dstV + dstOffsetUV) = componentLut[*(srcV + tileCtrUV)];

            finalY = componentLut[*(srcY + tileCtrY)];
              
            *(dstY + dstOffsetY) =                   finalY;
            *(dstY + dstOffsetY + 1) =               finalY;
            *(dstY + dstOffsetY + DST_PITCH_Y) =     finalY;
            *(dstY + dstOffsetY + DST_PITCH_Y + 1) = finalY;
            break;

          default :
            env->ThrowError("TurnsTile: Invalid planar copyMode; "
                            "please contact me!");
            break;

          }
            
        }        

      }
                        		
    }

  }

  return dst;

}



// Global functions, I know, ick, but this is the simplest solution I could
// find for making these available to my Create_TurnsTile function. I said I
// was new to C++, didn't I? Cut a guy some slack.
int TurnsTile_mod(int num, int mod, int min, int max)
{

  int base =  static_cast<int>  (
                ( static_cast<double> (num) /
                  static_cast<double> (mod) ) + 0.5
              ) * mod;

  return base >= max ? max : base <= min ? min : base;

}



int TurnsTile_gcf(int a, int b) {  

  if (b == 0)
    return a;
  else
    return TurnsTile_gcf(b, a % b);

}