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



#include <string>

#include "../../include/catch/catch.hpp"

#include "util_avsavx.h"



TEST_CASE(
  "TurnsTile - Tile dimension at multiples of minimum produces expected result",
  "[output][turnstile][tilew][tileh][multiple][minimum]")
{

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

#else

  std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

  int count = 4;

#endif

  std::string mults[4] = { "_wodd_hodd", "_wodd_heven",
                           "_weven_hodd", "_weven_heven" };

  for (int i = 0; i < count; ++i) {

    for (int j = 0; j < 4; ++j) {

      RunTestAvs("output-turnstile-tilew-tileh-multiple-minimum_clip_" +
                  csps[i] + mults[j]);
      RunTestAvs("output-turnstile-tilew-tileh-multiple-minimum_tilesheet_" +
                  csps[i] + mults[j]);

    }

  }

}
