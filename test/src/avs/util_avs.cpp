#include "util_avs.h"

#include <string>
#include <vector>

#include "../../../src/interface.h"
#include "../util_common.h"



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
  std::string runTestScript = "function TurnsTileRunTest(string filename)"
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

  AVSValue result = env->Invoke("TurnsTileRunTest", AVSValue(script.c_str()));

  std::string dataCur;

  if (result.IsString()) {

    dataCur = SplitError(result.AsString());

  } else if (result.IsClip()) {

    PVideoFrame frm = result.AsClip()->GetFrame(0, env);

    std::vector<plane> planes;

    plane y, u, v;

    y.ptr = frm->GetReadPtr(PLANAR_Y);
    y.pitch = frm->GetPitch(PLANAR_Y);
    y.row_size = frm->GetRowSize(PLANAR_Y);
    y.height = frm->GetHeight(PLANAR_Y);
    planes.push_back(y);

    u.ptr = frm->GetReadPtr(PLANAR_U);
    u.pitch = frm->GetPitch(PLANAR_U);
    u.row_size = frm->GetRowSize(PLANAR_U);
    u.height = frm->GetHeight(PLANAR_U);
    if (u.ptr)
      planes.push_back(u);

    v.ptr = frm->GetReadPtr(PLANAR_V);
    v.pitch = frm->GetPitch(PLANAR_V);
    v.row_size = frm->GetRowSize(PLANAR_V);
    v.height = frm->GetHeight(PLANAR_V);
    if (v.ptr)
      planes.push_back(v);

    dataCur = GetFrameHash(planes);

  } else {

    dataCur = "Error evaluating script " + script + "!";

  }

  CompareData(dataCur, refDir + name + ".txt");

}
