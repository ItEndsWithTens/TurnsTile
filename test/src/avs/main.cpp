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



#include "Windows.h"

#include <string>
#include <vector>

#define CATCH_CONFIG_RUNNER
#include "../../include/catch/catch.hpp"

#include "../../../src/interface.h"



const AVS_Linkage* AVS_linkage = 0;



IScriptEnvironment* env = 0;

bool writeRefData = false;

std::string testRoot = "./", scriptDir = "", refDir = "";



int main(int argc, char* argv[])
{

  // The Catch test framework will eventually support custom command line
  // options, but the feature isn't finished or documented yet, so for now I'm
  // manually parsing argv and setting a global to generate reference data.
  std::vector<char*> args;

  for (int i = 0; i < argc; ++i) {

    std::string arg = argv[i];

    if (arg == "--writeRefData") {

      writeRefData = true;

    } else if (arg.find("--testRoot") != std::string::npos) {

      testRoot = arg.substr(arg.find('=') + 1, std::string::npos);
      if (*testRoot.rbegin() != '/' && *testRoot.rbegin() != '\\')
        testRoot.push_back('/');

    } else {

      args.push_back(argv[i]);

    }

  }

  if (writeRefData == true)
    argc--;
  if (testRoot != ".")
    argc--;

  scriptDir = testRoot + "scripts/avs/";
  refDir = testRoot + "ref/avs/";


  typedef IScriptEnvironment* (__stdcall *CSE)(int);

  const char* libname = "avisynth";

  HMODULE lib = LoadLibrary(libname);
  if (!lib) {
    std::cerr << "Couldn't load Avisynth!" << std::endl;
    return -1;
  }

  CSE makeEnv = (CSE)GetProcAddress(lib, "CreateScriptEnvironment");
  if (!makeEnv) {
    std::cerr << "Couldn't find CreateScriptEnvironment function!" << std::endl;
    return -1;
  }

  env = makeEnv(AVISYNTH_INTERFACE_VERSION);
  if (!env) {
    std::cerr << "Couldn't create script environment!" << std::endl;
    return -1;
  }

  AVS_linkage = env->GetAVSLinkage();

  try {

    const char* ver = env->Invoke("VersionString", AVSValue(0, 0)).AsString();
    std::cout << "Plugin host: " << ver << std::endl << std::endl;

  } catch (AvisynthError& err) {

    std::cerr << err.msg << std::endl;
    return -1;

  }

  int result = Catch::Session().run(argc, args.data());


  if (env)
    env->DeleteScriptEnvironment();

  if (lib)
    FreeLibrary(lib);

  return result;

}
