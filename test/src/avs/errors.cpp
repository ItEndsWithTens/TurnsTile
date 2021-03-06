#include <string>

#include "../../include/catch/catch.hpp"

#include "util_avs.h"



TEST_CASE(
  "TurnsTile - Colorspace mismatch throws expected error",
  "[errors][turnstile][colorspace][mismatch]")
{

  RunTestAvs("errors-turnstile-colorspace-mismatch");

}



TEST_CASE(
  "TurnsTile - Interlaced clip height not mod minimum throws expected error",
  "[errors][turnstile][interlaced][height][mod]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  for (int i = 0; i < count; ++i) {

    RunTestAvs(
      "errors-turnstile-interlaced-height-mod-" + csps[i] + "_clip");
    RunTestAvs(
      "errors-turnstile-interlaced-height-mod-" + csps[i] + "_tilesheet");

  }

}



TEST_CASE(
  "TurnsTile - Tile width less than minimum throws expected error",
  "[errors][turnstile][tile][width][minimum]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  for (int i = 0; i < count; ++i)
    RunTestAvs("errors-turnstile-tile-width-minimum-" + csps[i]);

}



TEST_CASE(
  "TurnsTile - Tile height less than minimum throws expected error",
  "[errors][turnstile][tile][height][minimum]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  for (int i = 0; i < count; ++i) {

    RunTestAvs(
      "errors-turnstile-tile-height-minimum-" + csps[i] + "_progressive");
    RunTestAvs(
      "errors-turnstile-tile-height-minimum-" + csps[i] + "_interlaced");

  }

}



TEST_CASE(
  "TurnsTile - Tile width greater than maximum throws expected error",
  "[errors][turnstile][tile][width][maximum]")
{

  RunTestAvs("errors-turnstile-tile-width-maximum_clip");
  RunTestAvs("errors-turnstile-tile-width-maximum_tilesheet");

}



TEST_CASE(
  "TurnsTile - Tile height greater than maximum throws expected error",
  "[errors][turnstile][tile][height][maximum]")
{

  RunTestAvs("errors-turnstile-tile-height-maximum_clip");
  RunTestAvs("errors-turnstile-tile-height-maximum_tilesheet");

}



TEST_CASE(
  "TurnsTile - Tile width not mod minimum throws expected error",
  "[errors][turnstile][tile][width][mod]")
{

  std::string csps[4] = { "yuy2", "yv12", "yv16", "yv411" };

  int count = 4;

  for (int i = 0; i < count; ++i)
    RunTestAvs("errors-turnstile-tile-width-mod-" + csps[i]);

}



TEST_CASE(
  "TurnsTile - Tile height not mod minimum throws expected error",
  "[errors][turnstile][tile][height][mod]")
{

  RunTestAvs("errors-turnstile-tile-height-mod_progressive");

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  for (int i = 0; i < count; ++i)
    RunTestAvs("errors-turnstile-tile-height-mod-" + csps[i] + "_interlaced");

}



TEST_CASE(
  "TurnsTile - Tile width that's not a factor of maximum throws expected error",
  "[errors][turnstile][tile][width][factor]")
{

  RunTestAvs("errors-turnstile-tile-width-factor_clip");
  RunTestAvs("errors-turnstile-tile-width-factor_tilesheet");

}



TEST_CASE(
  "TurnsTile - Tile height that's not a factor of maximum throws expected error",
  "[errors][turnstile][tile][height][factor]")
{

  RunTestAvs("errors-turnstile-tile-height-factor_clip");
  RunTestAvs("errors-turnstile-tile-height-factor_tilesheet");

}



TEST_CASE(
  "TurnsTile - Invalid mode throws expected error",
  "[errors][turnstile][mode][range]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  for (int i = 0; i < count; ++i) {

    RunTestAvs("errors-turnstile-mode-range-" + csps[i] + "_lessthanmin");
    RunTestAvs("errors-turnstile-mode-range-" + csps[i] + "_greaterthanmax");

  }

}



TEST_CASE(
  "TurnsTile - Invalid levels string throws expected error",
  "[errors][turnstile][levels]")
{

  RunTestAvs("errors-turnstile-levels");

}



TEST_CASE(
  "TurnsTile - Invalid lotile throws expected error",
  "[errors][turnstile][lotile][range]")
{

  RunTestAvs("errors-turnstile-lotile-range_clip_lessthanmin");
  RunTestAvs("errors-turnstile-lotile-range_clip_greaterthanmax");
  RunTestAvs("errors-turnstile-lotile-range_tilesheet_lessthanmin");
  RunTestAvs("errors-turnstile-lotile-range_tilesheet_greaterthanmax");

}



TEST_CASE(
  "TurnsTile - Invalid hitile throws expected error",
  "[errors][turnstile][hitile][range]")
{

  RunTestAvs("errors-turnstile-hitile-range_clip_lessthanmin");
  RunTestAvs("errors-turnstile-hitile-range_clip_greaterthanmax");
  RunTestAvs("errors-turnstile-hitile-range_tilesheet_lessthanmin");
  RunTestAvs("errors-turnstile-hitile-range_tilesheet_greaterthanmax");

}



TEST_CASE(
  "TurnsTile - Lotile greater than hitile throws expected error",
  "[errors][turnstile][lotile][hitile]")
{

  RunTestAvs("errors-turnstile-lotile-hitile");

}



TEST_CASE(
  "CLUTer - Colorspace mismatch in CLUTer throws expected error",
  "[errors][cluter][colorspace][mismatch]")
{

  RunTestAvs("errors-cluter-colorspace-mismatch");

}



TEST_CASE(
  "CLUTer - Interlaced clip height not mod minimum throws expected error",
  "[errors][cluter][interlaced][height][mod]")
{

  std::string csps[8] = { "rgb32", "rgb24", "yuy2", "yv12",
                          "yv24", "yv16", "yv411", "y8" };

  int count = 8;

  for (int i = 0; i < count; ++i)
    RunTestAvs("errors-cluter-interlaced-height-mod-" + csps[i]);

}
