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

#include "../../../src/interface.h"
#include "../util_common.h"



#ifdef TURNSTILE_HOST_AVXSYNTH

using avxsynth::AvisynthError;
using avxsynth::AVSValue;
using avxsynth::IScriptEnvironment;
using avxsynth::PVideoFrame;

#endif



extern IScriptEnvironment* env;

extern bool writeRefData;

extern std::string scriptDir, refDir;



AVSValue ImportScriptAvs(std::string script)
{

  AVSValue dataCur = "";

  try {

    AVSValue args[1] = { script.c_str() };
    dataCur = env->Invoke("Import", AVSValue(args, 1));

  } catch (IScriptEnvironment::NotFound&) {

    dataCur = "Could not find Import function!";

  } catch (AvisynthError& err) {

    dataCur = err.msg;

  }

  return dataCur;

}



void RunTestAvs(std::string name)
{

  AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

  std::string dataCur;

  if (result.IsString()) {

    dataCur = SplitError(result.AsString());

  } else {

    dataCur = "Test script did not produce string!";

  }

  CompareData(dataCur, refDir + name + ".txt");

}