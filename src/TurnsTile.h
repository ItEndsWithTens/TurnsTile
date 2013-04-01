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



#ifndef __TurnsTile_H__
#define __TurnsTile_H__
#endif



class TurnsTile : public GenericVideoFilter
{

public:
	
  TurnsTile(PClip _child, PClip _tileSheet, int _tileW, int _tileH, int _res, int _mode, IScriptEnvironment* env);
  ~TurnsTile();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

private:

  PClip tileSheet;

  int tileW, tileH, mode;
  double res;
    
  int srcCols, srcRows;
  int sheetCols, sheetRows, sheetTiles;
  int csp, wStep, bytesPerPixel, tileBytes;
  double idxRangeMax;
 
  int scaleToRange(double rawIdx, double res, double outMax);

};