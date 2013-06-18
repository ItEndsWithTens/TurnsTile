#ifndef TURNSTILE_SRC_INTERFACE_H_INCLUDED_1BECD44A8CD145EFA502D1D50B46924E
#define TURNSTILE_SRC_INTERFACE_H_INCLUDED_1BECD44A8CD145EFA502D1D50B46924E



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



#if defined(TURNSTILE_HOST_AVXSYNTH)
  // Avxsynth, ae45271710
  // https://github.com/avxsynth/avxsynth/blob/ae452717100592fa463b0ec90bc5b88c142f5254/include/avxplugin.h
  #include "../include/avx/avxplugin.h"
#else
  #include <Windows.h>
  #if defined(TURNSTILE_HOST_AVISYNTH_26)
    // SEt's 32 bit 2.6.0 MT, March 9th, 2013
    // http://forum.doom9.org/showthread.php?t=148782
    #include "../include/avs/avisynth-260MT-x86.h"
  #elif defined(TURNSTILE_HOST_AVISYNTH_25_X64)
    // JoshyD's avisynth64, r5
    // https://code.google.com/p/avisynth64/source/browse/trunk/src/core/avisynth.h?&r=5
    #include "../include/avs/avisynth-258MT-x64.h"
  #else
    // SEt's 32 bit 2.5.8 MT, July 12th, 2009
    // http://forum.doom9.org/showthread.php?t=148117
    #include "../include/avs/avisynth-258MT-x86.h"
  #endif
#endif



#endif // TURNSTILE_SRC_INTERFACE_H_INCLUDED_1BECD44A8CD145EFA502D1D50B46924E
