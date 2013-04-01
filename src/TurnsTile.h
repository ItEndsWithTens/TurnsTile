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



#ifndef __TurnsTile_H__
#define __TurnsTile_H__
#endif

#include <string>

using namespace std;



class TurnsTile : public GenericVideoFilter
{

public:
	
  TurnsTile(PClip _child, PClip _tileSheet, int _tileW, int _tileH, int _res, int _mode, AVSValue _levels, int _lotile, int _hitile, IScriptEnvironment* env);
  TurnsTile(PClip _child, int _tileW, int _tileH, int _res, int _mode, AVSValue _levels, int _lotile, int _hitile, IScriptEnvironment* env);
  
  ~TurnsTile();
  
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  
private:
  
  PClip tileSheet;

  bool userSheet;

  int tileW, tileH, mode, res,
      srcCols, srcRows,
      sheetCols, sheetRows, sheetTiles,
      csp, wStep, bytesPerPixel, tileBytes,
      idxInMin, idxInMax, idxOutMin, idxOutMax,
      depthStep, copyMode;

  double idxScaleFactor;

  string levels;
  
  int scaleToRange(int rawIdx, double scaleFactor, int idxOutMin, int idxOutMax, int res);
  int mod(int num, int mod, int min, int max);

};



int TurnsTile_gcf(int a, int b);