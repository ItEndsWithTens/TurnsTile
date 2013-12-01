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

using avxsynth::AVSValue;
using avxsynth::IScriptEnvironment;
using avxsynth::PVideoFrame;

#endif



extern IScriptEnvironment* env;

extern bool writeRefData;

extern std::string scriptDir, refDir;



void RunTestAvs(std::string name)
{

  // Using the Avisynth language's try...catch keywords, instead of using C++
  // exceptions directly, allows me to fix my mistake of catching exceptions
  // across a DLL boundary. Here, in the case of an error when Importing the
  // script name passed in to RunTest, Avisynth will throw and catch its own
  // exception, then return the error message as an AVSValue.
  //
  // I implement this script function as a string that's Evaled instead of an
  // .avs file, in an effort to ensure the function will always exist and always
  // work properly, which avoids trying to load a script and allowing my
  // supposedly safer approach to potentially throw an exception itself.
  std::string runTestScript = "function RunTest(string filename)"
                              "{"

                              "  try {"
                        
                              "    result = Import(filename)"

                              "  } catch (err) {"

                              "    result = err"

                              "  }"

                              "  return result"

                              "}";

  env->Invoke("Eval", AVSValue(runTestScript.c_str()));

  std::string script = scriptDir + name + ".avs";

  AVSValue result = env->Invoke("RunTest", AVSValue(script.c_str()));
  
  std::string dataCur;

  if (result.IsString()) {

    dataCur = SplitError(result.AsString());

  } else if (result.IsClip()) {

    PVideoFrame frm = result.AsClip()->GetFrame(0, env);
    const unsigned char* frmp = frm->GetReadPtr();
    const int PITCH = frm->GetPitch(),
              ROW_SIZE = frm->GetRowSize(),
              HEIGHT = frm->GetHeight();

    dataCur = GetFrameHash(frmp, ROW_SIZE, PITCH, HEIGHT);

  } else {

    dataCur = "Error evaluating script " + script + "!";

  }

  CompareData(dataCur, refDir + name + ".txt");

}