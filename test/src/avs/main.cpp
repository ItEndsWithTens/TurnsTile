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



#ifndef WIN32
#include <dlfcn.h>
#endif

#include <string>
#include <vector>

#define CATCH_CONFIG_RUNNER
#include "../../include/catch/catch.hpp"

#include "../../../src/interface.h"



const AVS_Linkage* AVS_linkage = nullptr;



IScriptEnvironment* env = 0;

bool writeRefData = false;

std::string testRoot = "../../../test/", scriptDir = "", refDir = "";



int main(int argc, char* argv[])
{

  Catch::Session session;

  using namespace Catch::clara;
  auto cli = session.cli() |
    Opt(testRoot, "testRoot")["--testRoot"]("the directory containing 'ref' and 'scripts'") |
    Opt(writeRefData)["--writeRefData"]("write test output to disk");

  session.cli(cli);

  int returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0)
    return returnCode;

  if (*testRoot.rbegin() != '/' && *testRoot.rbegin() != '\\')
    testRoot.push_back('/');

  scriptDir = testRoot + "scripts/avs/";
  refDir = testRoot + "ref/avs/";

  typedef IScriptEnvironment* (__stdcall *CSE)(int);

#if defined(WIN32)
  const char* libname = "avisynth";
#elif defined(__APPLE__)
  const char* libname = "libavisynth.dylib";
#else
  const char* libname = "libavisynth.so";
#endif

#ifdef WIN32
  HMODULE lib = LoadLibrary(libname);
#else
  void* lib = dlopen(libname, RTLD_LAZY);
#endif
  if (!lib) {
    std::cerr << "Couldn't load Avisynth!" << std::endl;
    return -1;
  }

#ifdef WIN32
  CSE makeEnv = (CSE)GetProcAddress(lib, "CreateScriptEnvironment");
#else
  CSE makeEnv = (CSE)dlsym(lib, "CreateScriptEnvironment");
#endif
  if (!makeEnv) {
    std::cerr << "Couldn't find CreateScriptEnvironment function!" << std::endl;
    return -1;
  }

  env = makeEnv(AVISYNTH_INTERFACE_VERSION);
  if (!env) {
    env = makeEnv(AVISYNTH_CLASSIC_INTERFACE_VERSION);
  }
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

  int result = session.run();

  if (env)
    env->DeleteScriptEnvironment();

  if (lib)
#ifdef WIN32
    FreeLibrary(lib);  
#else
    dlclose(lib);
#endif

  return result;

}
