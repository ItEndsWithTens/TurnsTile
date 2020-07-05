#ifndef TURNSTILE_SRC_TURNSTILEBMPSOURCE_H_INCLUDED_ED833916B6E847FA902728966474B382
#define TURNSTILE_SRC_TURNSTILEBMPSOURCE_H_INCLUDED_ED833916B6E847FA902728966474B382



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



#include <string>

#include "interface.h"



class TurnsTileBmpSource : public IClip
{

public:

  TurnsTileBmpSource(
    std::string filename, std::string pixel_type, IScriptEnvironment* env);
  ~TurnsTileBmpSource() {}

  void __stdcall GetAudio(
    void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {}
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return frm; }
  bool __stdcall GetParity(int n) { return false; }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  int __stdcall SetCacheHints(int cachehints, int frame_range) { return 0; }

private:

  PVideoFrame frm;
  VideoInfo vi;

};



AVSValue __cdecl Create_TurnsTileBmpSource(
  AVSValue args, void* user_data, IScriptEnvironment* env);

#endif // TURNSTILE_SRC_TURNSTILEBMPSOURCE_H_INCLUDED_ED833916B6E847FA902728966474B382
