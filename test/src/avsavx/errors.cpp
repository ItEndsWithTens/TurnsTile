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
#include "util_avsavx.h"
#include "../util_common.h"



#ifdef TURNSTILE_HOST_AVXSYNTH

using avxsynth::AvisynthError;
using avxsynth::AVSValue;
using avxsynth::IScriptEnvironment;
using avxsynth::PClip;

#endif



extern IScriptEnvironment* env;

extern bool writeRefData;

extern std::string scriptDir, refDir;



TEST_CASE(
  "TurnsTile - Colorspace mismatch throws expected error",
  "[errors][turnstile][colorspace][mismatch]")
{

  std::string name = "errors-turnstile-colorspace-mismatch";

  AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

  REQUIRE(result.IsString());

  std::string dataCur = SplitError(result.AsString()),
              dataRef = refDir + name + ".txt";

  CompareData(dataCur, dataRef);

}



TEST_CASE(
  "TurnsTile - Interlaced clip height not mod minimum throws expected error",
  "[errors][turnstile][interlaced][height][mod]")
{

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

#else

  std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

  int count = 4;

#endif

  SECTION("clip") {

    for (int i = 0; i < count; ++i) {

      std::string name = "errors-turnstile-interlaced-height-mod-" + csps[i] +
                         "_clip";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

  SECTION("tilesheet") {

    for (int i = 0; i < count; ++i) {

      std::string name = "errors-turnstile-interlaced-height-mod-" + csps[i] +
                         "_tilesheet";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

}



TEST_CASE(
  "TurnsTile - Tile width less than minimum throws expected error",
  "[errors][turnstile][tile][width][minimum]")
{

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

#else

  std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

  int count = 4;

#endif

  for (int i = 0; i < count; ++i) {

    std::string name = "errors-turnstile-tile-width-minimum-" + csps[i];

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

  }

}



TEST_CASE(
  "TurnsTile - Tile height less than minimum throws expected error",
  "[errors][turnstile][tile][height][minimum]")
{

  SECTION("progressive") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                            "yv24", "yv16", "yv411", "y8" };

    int count = 8;

#else

    std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

    int count = 4;

#endif

    for (int i = 0; i < count; ++i) {

      std::string name = "errors-turnstile-tile-height-minimum-" + csps[i] +
                         "_progressive";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

  SECTION("interlaced", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                            "yv24", "yv16", "yv411", "y8" };

    int count = 8;

#else

    std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

    int count = 4;

#endif

    for (int i = 0; i < count; ++i) {

      std::string name = "errors-turnstile-tile-height-minimum-" + csps[i] +
                         "_interlaced";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

}



TEST_CASE(
  "TurnsTile - Tile width greater than maximum throws expected error",
  "[errors][turnstile][tile][width][maximum]")
{

  SECTION("clip") {

    std::string name = "errors-turnstile-tile-width-maximum_clip";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

  SECTION("tilesheet") {

    std::string name = "errors-turnstile-tile-width-maximum_tilesheet";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

}



TEST_CASE(
  "TurnsTile - Tile height greater than maximum throws expected error",
  "[errors][turnstile][tile][height][maximum]")
{

  SECTION("clip", "") {

    std::string name = "errors-turnstile-tile-height-maximum_clip";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

  SECTION("tilesheet", "") {

    std::string name = "errors-turnstile-tile-height-maximum_tilesheet";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

}



TEST_CASE(
  "TurnsTile - Tile width not mod minimum throws expected error",
  "[errors][turnstile][tile][width][mod]")
{

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string csps[4] = { "yuy2", "yv12", "yv16", "yv411" };

  int count = 4;

#else

  std::string csps[2] = { "yuy2", "yv12" };

  int count = 2;

#endif

  for (int i = 0; i < count; ++i) {

    std::string name = "errors-turnstile-tile-width-mod-" + csps[i];

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

}



TEST_CASE(
  "TurnsTile - Tile height not mod minimum throws expected error",
  "[errors][turnstile][tile][height][mod]")
{

  SECTION("progressive") {

    std::string name = "errors-turnstile-tile-height-mod_progressive";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

  SECTION("interlaced") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                            "yv24", "yv16", "yv411", "y8" };

    int count = 8;

#else

    std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

    int count = 4;

#endif

    for (int i = 0; i < count; ++i) {

      std::string name = "errors-turnstile-tile-height-mod-" + csps[i] +
                         "_interlaced";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

}



TEST_CASE(
  "TurnsTile - Tile width that's not a factor of maximum throws expected error",
  "[errors][turnstile][tile][width][factor]")
{

  SECTION("clip", "") {

    std::string name = "errors-turnstile-tile-width-factor_clip";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

  SECTION("tilesheet", "") {

    std::string name = "errors-turnstile-tile-width-factor_tilesheet";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

}



TEST_CASE(
  "TurnsTile - Tile height that's not a factor of maximum throws expected error",
  "[errors][turnstile][tile][height][factor]")
{

  SECTION("clip") {

    std::string name = "errors-turnstile-tile-height-factor_clip";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

  SECTION("tilesheet", "") {

    std::string name = "errors-turnstile-tile-height-factor_tilesheet";

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

}



TEST_CASE(
  "TurnsTile - Invalid mode throws expected error",
  "[errors][turnstile][mode][range]")
{

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                            "yv24", "yv16", "yv411", "y8" };

    int count = 8;

#else

    std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

    int count = 4;

#endif

  SECTION("lessthanmin") {

    for (int i = 0; i < count; ++i) {

      std::string name = "errors-turnstile-mode-range-" + csps[i] +
                         "_lessthanmin";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

  SECTION("greaterthanmax") {

    for (int i = 0; i < count; ++i) {

      std::string name = "errors-turnstile-mode-range-" + csps[i] +
                         "_greaterthanmax";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

}



TEST_CASE(
  "TurnsTile - Invalid levels string throws expected error",
  "[errors][turnstile][levels]")
{

  std::string name = "errors-turnstile-levels";

  AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

  REQUIRE(result.IsString());

  std::string dataCur = SplitError(result.AsString()),
              dataRef = refDir + name + ".txt";

  CompareData(dataCur, dataRef);

}



TEST_CASE(
  "TurnsTile - Invalid lotile throws expected error",
  "[errors][turnstile][lotile][range]")
{

  SECTION("clip") {

    SECTION("lessthanmin") {

      std::string name = "errors-turnstile-lotile-range_clip_lessthanmin";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

    SECTION("greaterthanmax") {

      std::string name = "errors-turnstile-lotile-range_clip_greaterthanmax";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

  SECTION("tilesheet") {

    SECTION("lessthanmin") {

      std::string name = "errors-turnstile-lotile-range_tilesheet_lessthanmin";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

    SECTION("greaterthanmax") {

      std::string name = "errors-turnstile-lotile-range_tilesheet_greaterthanmax";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

}



TEST_CASE(
  "TurnsTile - Invalid hitile throws expected error",
  "[errors][turnstile][hitile][range]")
{

  SECTION("clip") {

    SECTION("lessthanmin") {

      std::string name = "errors-turnstile-hitile-range_clip_lessthanmin";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

    SECTION("greaterthanmax") {

      std::string name = "errors-turnstile-hitile-range_clip_greaterthanmax";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

  SECTION("tilesheet") {

    SECTION("lessthanmin") {

      std::string name = "errors-turnstile-hitile-range_tilesheet_lessthanmin";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

    SECTION("greaterthanmax") {

      std::string name = "errors-turnstile-hitile-range_tilesheet_greaterthanmax";

      AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

      REQUIRE(result.IsString());

      std::string dataCur = SplitError(result.AsString()),
                  dataRef = refDir + name + ".txt";

      CompareData(dataCur, dataRef);

    }

  }

}



TEST_CASE(
  "TurnsTile - Lotile greater than hitile throws expected error",
  "[errors][turnstile][lotile][hitile]")
{

  std::string name = "errors-turnstile-lotile-hitile";

  AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

  REQUIRE(result.IsString());

  std::string dataCur = SplitError(result.AsString()),
              dataRef = refDir + name + ".txt";

  CompareData(dataCur, dataRef);

}



TEST_CASE(
  "CLUTer - Colorspace mismatch in CLUTer throws expected error",
  "[errors][cluter][colorspace][mismatch]")
{

  std::string name = "errors-cluter-colorspace-mismatch";

  AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

  REQUIRE(result.IsString());

  std::string dataCur = SplitError(result.AsString()),
              dataRef = refDir + name + ".txt";

  CompareData(dataCur, dataRef);

}



TEST_CASE(
  "CLUTer - Interlaced clip height not mod minimum throws expected error",
  "[errors][cluter][interlaced][height][mod]")
{

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

#else

  std::string csps[4] = { "rgb32", "rgb24", "yuy2", "yv12" };

  int count = 4;

#endif

  for (int i = 0; i < count; ++i) {

    std::string name = "errors-cluter-interlaced-height-mod-" + csps[i];

    AVSValue result = ImportScriptAvs(scriptDir + name + ".avs");

    REQUIRE(result.IsString());

    std::string dataCur = SplitError(result.AsString()),
                dataRef = refDir + name + ".txt";

    CompareData(dataCur, dataRef);

  }

}
