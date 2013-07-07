#ifndef TURNSTILE_TEST_SRC_AVSAVX_UTIL_AVSAVX_H_INCLUDED_E8B57355D78349508228C48B2A07D8F6
#define TURNSTILE_TEST_SRC_AVSAVX_UTIL_AVSAVX_H_INCLUDED_E8B57355D78349508228C48B2A07D8F6



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

#include "../../../src/interface.h"



#ifdef TURNSTILE_HOST_AVXSYNTH

#define AVSValue avxsynth::AVSValue
#define IScriptEnvironment avxsynth::IScriptEnvironment

#endif



AVSValue ImportScriptAvs(std::string script);

void RunTestAvs(std::string name);



#ifdef TURNSTILE_HOST_AVXSYNTH

#undef AVSValue
#undef IScriptEnvironment

#endif



#endif // TURNSTILE_TEST_SRC_AVSAVX_UTIL_AVSAVX_H_INCLUDED_E8B57355D78349508228C48B2A07D8F6
