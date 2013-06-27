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



#include "../../src/interface.h"

#ifdef WIN32
  #include "Windows.h"
#else
  #include <dlfcn.h>
#endif

#define CATCH_CONFIG_RUNNER
#include "../catch/catch.hpp"



#ifdef TURNSTILE_HOST_AVXSYNTH

using avxsynth::AVISYNTH_INTERFACE_VERSION;
using avxsynth::AvisynthError;
using avxsynth::AVSValue;
using avxsynth::IScriptEnvironment;
using avxsynth::PClip;

#endif



#ifdef TURNSTILE_HOST_AVISYNTH_26

const AVS_Linkage* AVS_linkage = 0;

#endif



IScriptEnvironment* env = 0;



int main(int argc, char* const argv[])
{

  typedef IScriptEnvironment* (__stdcall *CSE)(int);

#if defined(WIN32)
  const char* libname = "avisynth";
#elif defined(__APPLE__)
  const char* libname = "libavxsynth.dylib";
#else
  const char* libname = "libavxsynth.so";
#endif

#ifdef WIN32
  HMODULE lib = LoadLibrary(libname);
#else
  void* lib = dlopen(libname, RTLD_LAZY);
#endif
  if(!lib) {
    std::cerr << "Couldn't load Avisynth!" << std::endl;
    return -1;
  }

#ifdef WIN32
  CSE makeEnv = (CSE)GetProcAddress(lib, "CreateScriptEnvironment");
#else
  CSE makeEnv = (CSE)dlsym(lib, "CreateScriptEnvironment");
#endif
  if(!makeEnv) {
    std::cerr << "Couldn't find CreateScriptEnvironment function!" << std::endl;
    return -1;
  }

  env = makeEnv(AVISYNTH_INTERFACE_VERSION);
  if(!env) {
    std::cerr << "Couldn't create script environment!" << std::endl;
    return -1;
  }

#ifdef TURNSTILE_HOST_AVISYNTH_26
  AVS_linkage = env->GetAVSLinkage();
#endif

  try {

    const char* ver = env->Invoke("VersionString", AVSValue(0, 0)).AsString();
    std::cout << "Plugin host: " << ver << std::endl << std::endl;

  } catch (AvisynthError& err) {

    std::cout << err.msg << std::endl;
    return -1;

  }

  if(!env->FunctionExists("TurnsTile")) {
    std::cerr << "Couldn't find TurnsTile function!" << std::endl;
    return -1;
  }

  if(!env->FunctionExists("CLUTer")) {
    std::cerr << "Couldn't find CLUTer function!" << std::endl;
    return -1;
  }


  int ret = Catch::Main(argc, argv);


  if (env)
#if AVISYNTH_INTERFACE_VERSION >= 5
    env->DeleteScriptEnvironment();
#else
    delete env;
#endif

  if (lib)
#ifdef WIN32
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif

  return ret;

}
