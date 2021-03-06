#include <string>

#include "../../include/catch/catch.hpp"

#include "util_avs.h"



TEST_CASE(
  "TurnsTile - Tile dimension at multiples of minimum produces expected result",
  "[output][turnstile][tilew][tileh][multiple][minimum]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  std::string mults[4] = { "_wodd_hodd", "_wodd_heven",
                           "_weven_hodd", "_weven_heven" };

  for (int i = 0; i < count; ++i) {

    for (int j = 0; j < 4; ++j) {

      RunTestAvs("output-turnstile-tilew-tileh-multiple-minimum_clip_" +
                  csps[i] + mults[j]);
      RunTestAvs("output-turnstile-tilew-tileh-multiple-minimum_tilesheet_" +
                  csps[i] + mults[j]);

    }

  }

}



TEST_CASE(
  "TurnsTile - Simulation of reduced bit depth produces expected results",
  "[output][turnstile][res]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  std::string res[3] = { "_0", "_1", "_8" };

  for (int i = 0; i < count; ++i) {

    for (int j = 0; j < 3; ++j) {

      RunTestAvs("output-turnstile-res_clip_" + csps[i] + res[j]);
      RunTestAvs("output-turnstile-res_tilesheet_" + csps[i] + res[j]);

    }

  }

}



TEST_CASE(
  "TurnsTile - Using TV levels with a tilesheet produces expected results",
  "[output][turnstile][levels]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  for (int i = 0; i < count; ++i)
    RunTestAvs("output-turnstile-levels_" + csps[i]);

}



TEST_CASE(
  "TurnsTile - Mode parameter produces expected results",
  "[output][turnstile][mode]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" },
              modes[7] = { "_0", "_1", "_2", "_3", "_4", "_5", "_6" };

  int modesPerCsp[8] = { 5, 4, 5, 7, 4, 5, 7, 2 };

  int countCsps = 8;

  for (int i = 0; i < countCsps; ++i)
    for (int j = 0; j < modesPerCsp[i]; ++j)
      RunTestAvs("output-turnstile-mode_" + csps[i] + modes[j]);

}



TEST_CASE(
  "TurnsTile - Lotile and hitile parameters produce expected results",
  "[output][turnstile][lotile][hitile]")
{

  RunTestAvs("output-turnstile-lotile-hitile_clip");
  RunTestAvs("output-turnstile-lotile-hitile_tilesheet");

}



TEST_CASE(
  "TurnsTile - Interlaced parameter produces expected results",
  "[output][turnstile][interlaced]")
{

  RunTestAvs("output-turnstile-interlaced_clip");
  RunTestAvs("output-turnstile-interlaced_tilesheet");

}



TEST_CASE(
  "CLUTer - Paletteframe parameter produces expected results",
  "[output][cluter][paletteframe]")
{

  RunTestAvs("output-cluter-paletteframe_0");
  RunTestAvs("output-cluter-paletteframe_1");

}



TEST_CASE(
  "CLUTer - Interlaced parameter produces expected results",
  "[output][cluter][interlaced]")
{

  RunTestAvs("output-cluter-interlaced");

}
