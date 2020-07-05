#ifndef TURNSTILE_TEST_SRC_UTIL_COMMON_H_INCLUDED_2B23CA23607A4347BD1946126C3F0A39
#define TURNSTILE_TEST_SRC_UTIL_COMMON_H_INCLUDED_2B23CA23607A4347BD1946126C3F0A39



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
#include <vector>



struct plane
{

  const unsigned char* ptr;
  int row_size, pitch, height;

};



std::string GetFrameHash(std::vector<plane> planes);



std::string ReadRefData(std::string filename);



std::string SplitError(std::string err);



int WriteRefData(std::string hash, std::string filename);



void CompareData(std::string data, std::string filename);



#endif // TURNSTILE_TEST_SRC_UTIL_COMMON_H_INCLUDED_2B23CA23607A4347BD1946126C3F0A39
