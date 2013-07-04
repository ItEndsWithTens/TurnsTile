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



#include "../../../src/interface.h"

#include <string>

#include "../../include/catch/catch.hpp"



#ifdef TURNSTILE_HOST_AVXSYNTH

using avxsynth::AvisynthError;
using avxsynth::AVSValue;
using avxsynth::IScriptEnvironment;
using avxsynth::PClip;

#endif



extern IScriptEnvironment* env;



TEST_CASE("Errors/Turnstile/ColorspaceMismatch", "") {

  PClip clip = 0, tilesheet = 0;

  try {

    AVSValue args[1] = { "RGB32" };
    const char* names[1] = { "pixel_type" };
    clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

  } catch (AvisynthError& err) {

    FAIL(err.msg);

  }

  try {

    AVSValue args[1] = { "YV12" };
    const char* names[1] = { "pixel_type" };
    tilesheet = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

  } catch (AvisynthError& err) {

    FAIL(err.msg);

  }

  AVSValue out;

  try {

    AVSValue args[2] = { clip, tilesheet };
    out = env->Invoke("TurnsTile", AVSValue(args, 2)).AsClip();

  } catch (AvisynthError& err) {

    out = err.msg;

  }

  REQUIRE(out.IsString());
  CHECK(std::string(out.AsString()) ==
        "TurnsTile: clip and tilesheet must share a colorspace!");

}



TEST_CASE("Errors/Turnstile/InterlacedHeightMod", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string
    csps[8] = { "RGB32", "RGB24", "YUY2", "YV12",
                "YV24", "YV16", "YV411", "Y8" },
    mods[8] = { "2", "2", "2", "4", "2", "2", "2", "2" };

  int heights[8] = { 479, 479, 479, 478, 479, 479, 479, 479 };

  int count = 8;

#else

  std::string
    csps[4] = { "RGB32", "RGB24", "YUY2", "YV12" },
    mods[4] = { "2", "2", "2", "4" };

  int heights[4] = { 479, 479, 479, 478 };

  int count = 4;

#endif

  SECTION("Clip", "") {

    for (int i = 0; i < count; ++i) {

      PClip clip = 0;

      try {

        AVSValue args[2] = { heights[i], csps[i].c_str() };
        const char* names[2] = { "height", "pixel_type" };
        clip = env->Invoke("BlankClip", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        FAIL(err.msg);

      }

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, true };
        const char* names[2] = { 0, "interlaced" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: " + csps[i] + " clip height must be mod " + mods[i] +
            " when interlaced=true!");

    }

  }

  SECTION("Tilesheet", "") {

    for (int i = 0; i < count; ++i) {

      PClip clip = 0;

      try {

        AVSValue args[2] = { 480, csps[i].c_str() };
        const char* names[2] = { "height", "pixel_type" };
        clip = env->Invoke("BlankClip", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        FAIL(err.msg);

      }

      PClip tilesheet = 0;

      try {

        AVSValue args[2] = { heights[i], csps[i].c_str() };
        const char* names[2] = { "height", "pixel_type" };
        tilesheet = env->Invoke("BlankClip", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        FAIL(err.msg);

      }

      AVSValue out = 0;

      try {

        AVSValue args[3] = { clip, tilesheet, true };
        const char* names[3] = { 0, 0, "interlaced" };
        out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: " + csps[i] + " tilesheet height must be mod " +
            mods[i] + " when interlaced=true!");

    }

  }

}



TEST_CASE("Errors/Turnstile/TileWidthMinimum", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string
    csps[8] = { "RGB32", "RGB24", "YUY2", "YV12",
                "YV24", "YV16", "YV411", "Y8" },
    minsMsg[8] = { "1", "1", "2", "2", "1", "2", "4", "1" };

  int count = 8;

#else

  std::string
    csps[4] = { "RGB32", "RGB24", "YUY2", "YV12" },
    minsMsg[4] = { "1", "1", "2", "2" };

  int count = 4;

#endif

  for (int i = 0; i < count; ++i) {

    PClip clip = 0;

    try {

      AVSValue args[1] = { csps[i].c_str() };
      const char* names[1] = { "pixel_type" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[2] = { clip, 0 };
      const char* names[2] = { 0, "tilew" };
      out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: tilew must be at least " + minsMsg[i] + " for " +
          csps[i] + " input!");

  }

}



TEST_CASE("Errors/Turnstile/TileHeightMinimum", "") {

  SECTION("Progressive", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string
      csps[8] = { "RGB32", "RGB24", "YUY2", "YV12",
                  "YV24", "YV16", "YV411", "Y8" },
      minsMsg[8] = { "1", "1", "1", "2", "1", "1", "1", "1" };

    int count = 8;

#else

    std::string
      csps[4] = { "RGB32", "RGB24", "YUY2", "YV12" },
      minsMsg[4] = { "1", "1", "1", "2" };

    int count = 4;

#endif

    for (int i = 0; i < count; ++i) {

      PClip clip = 0;

      try {

        AVSValue args[1] = { csps[i].c_str() };
        const char* names[1] = { "pixel_type" };
        clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

      } catch (AvisynthError& err) {

        FAIL(err.msg);

      }

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, 0 };
        const char* names[2] = { 0, "tileh" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: tileh must be at least " + minsMsg[i] + " for " +
            csps[i] + " input!");

    }

  }

  SECTION("Interlaced", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string
      csps[8] = { "RGB32", "RGB24", "YUY2", "YV12",
                  "YV24", "YV16", "YV411", "Y8" },
      minsMsg[8] = { "2", "2", "2", "4", "2", "2", "2", "2" };

    int count = 8;

#else

    std::string
      csps[4] = { "RGB32", "RGB24", "YUY2", "YV12" },
      minsMsg[4] = { "2", "2", "2", "4" };

    int count = 4;

#endif

    for (int i = 0; i < count; ++i) {

      PClip clip = 0;

      try {

        AVSValue args[1] = { csps[i].c_str() };
        const char* names[1] = { "pixel_type" };
        clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

      } catch (AvisynthError& err) {

        FAIL(err.msg);

      }

      AVSValue out = 0;

      try {

        AVSValue args[3] = { clip, 0, true };
        const char* names[3] = { 0, "tileh", "interlaced" };
        out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: tileh must be at least " + minsMsg[i] +
            " for interlaced " + csps[i] + " input!");

    }

  }

}



TEST_CASE("Errors/Turnstile/TileWidthMaximum", "") {

  SECTION("NoTilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 640 };
      const char* names[1] = { "width" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[2] = { clip, 700 };
      const char* names[2] = { 0, "tilew" };
      out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tilew must not exceed 640!");

  }

  SECTION("Tilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 640 };
      const char* names[1] = { "width" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    PClip tilesheet = 0;

    try {

      AVSValue args[1] = { 256 };
      const char* names[1] = { "width" };
      tilesheet = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[3] = { clip, tilesheet, 700 };
      const char* names[3] = { 0, 0, "tilew" };
      out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tilew must not exceed 128!");

  }

}



TEST_CASE("Errors/Turnstile/TileHeightMaximum", "") {

  SECTION("NoTilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 480 };
      const char* names[1] = { "height" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[2] = { clip, 500 };
      const char* names[2] = { 0, "tileh" };
      out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tileh must not exceed 480!");

  }

  SECTION("Tilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 480 };
      const char* names[1] = { "height" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    PClip tilesheet = 0;

    try {

      AVSValue args[1] = { 256 };
      const char* names[1] = { "height" };
      tilesheet = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[3] = { clip, tilesheet, 500 };
      const char* names[3] = { 0, 0, "tileh" };
      out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tileh must not exceed 32!");

  }

}



TEST_CASE("Errors/Turnstile/TileWidthMod", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string
    csps[4] = { "YUY2", "YV12", "YV16", "YV411" },
    modsMsg[4] = { "2", "2", "2", "4" };

  int count = 4;

#else

  std::string
    csps[2] = { "YUY2", "YV12" },
    modsMsg[2] = { "2", "2" };

  int count = 2;

#endif

  for (int i = 0; i < count; ++i) {

    PClip clip = 0;

    try {

      AVSValue args[1] = { csps[i].c_str() };
      const char* names[1] = { "pixel_type" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[2] = { clip, 5 };
      const char* names[2] = { 0, "tilew" };
      out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For " + csps[i] +
          " input, tilew must be a multiple of " + modsMsg[i] + "!");

  }

}



TEST_CASE("Errors/Turnstile/TileHeightMod", "") {

  SECTION("Progressive", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { "YV12" };
      const char* names[1] = { "pixel_type" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[2] = { clip, 3 };
      const char* names[2] = { 0, "tileh" };
      out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For YV12 input, tileh must be a multiple of 2!");

  }

  SECTION("Interlaced", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string
      csps[8] = { "RGB32", "RGB24", "YUY2", "YV12",
                  "YV24", "YV16", "YV411", "Y8" },
      modsMsg[8] = { "2", "2", "2", "4", "2", "2", "2", "2" };

    int count = 8;

#else

    std::string
      csps[4] = { "RGB32", "RGB24", "YUY2", "YV12" },
      modsMsg[4] = { "2", "2", "2", "4" };

    int count = 4;

#endif

    for (int i = 0; i < count; ++i) {

      PClip clip = 0;

      try {

        AVSValue args[1] = { csps[i].c_str() };
        const char* names[1] = { "pixel_type" };
        clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

      } catch (AvisynthError& err) {

        FAIL(err.msg);

      }

      AVSValue out = 0;

      try {

        AVSValue args[3] = { clip, 5, true };
        const char* names[3] = { 0, "tileh", "interlaced" };
        out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: For interlaced " + csps[i] +
            " input, tileh must be a multiple of " + modsMsg[i] + "!");

    }

  }

}



TEST_CASE("Errors/Turnstile/TileWidthFactor", "") {

  SECTION("NoTilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 640 };
      const char* names[1] = { "width" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[2] = { clip, 17 };
      const char* names[2] = { 0, "tilew" };
      out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tilew must be a factor of 640!");

  }

  SECTION("Tilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 640 };
      const char* names[1] = { "width" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    PClip tilesheet = 0;

    try {

      AVSValue args[1] = { 256 };
      const char* names[1] = { "width" };
      tilesheet = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[3] = { clip, tilesheet, 19 };
      const char* names[3] = { 0, 0, "tilew" };
      out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tilew must be a factor of 128!");

  }

}



TEST_CASE("Errors/Turnstile/TileHeightFactor", "") {

  SECTION("NoTilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 480 };
      const char* names[1] = { "height" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[2] = { clip, 17 };
      const char* names[2] = { 0, "tileh" };
      out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tileh must be a factor of 480!");

  }

  SECTION("Tilesheet", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { 480 };
      const char* names[1] = { "height" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    PClip tilesheet = 0;

    try {

      AVSValue args[1] = { 256 };
      const char* names[1] = { "height" };
      tilesheet = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[3] = { clip, tilesheet, 17 };
      const char* names[3] = { 0, 0, "tileh" };
      out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "TurnsTile: For this input, tileh must be a factor of 32!");

  }

}



TEST_CASE("Errors/Turnstile/ModeRange", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

  SECTION("Y8", "") {

    PClip clip = 0;

    try {

      AVSValue args[1] = { "Y8" };
      const char* names[1] = { "pixel_type" };
      clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    SECTION("LessThanMin", "") {

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, -1 };
        const char* names[2] = { 0, "mode" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) == "TurnsTile: Y8 only allows mode 0!");

    }

    SECTION("GreaterThanMax", "") {

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, 1 };
        const char* names[2] = { 0, "mode" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) == "TurnsTile: Y8 only allows mode 0!");

    }

  }

#endif

  SECTION("NotY8", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

    std::string
      csps[7] = { "RGB32", "RGB24", "YUY2", "YV12",
                  "YV24", "YV16", "YV411" },
      modeMaxesMsg[7] = { "4", "3", "4", "6", "3", "4", "6" };

    int count = 7;

#else

    std::string
      csps[4] = { "RGB32", "RGB24", "YUY2", "YV12" },
      modeMaxesMsg[4] = { "4", "3", "4", "6" };

    int count = 4;

#endif

    SECTION("LessThanMin", "") {

      for (int i = 0; i < count; ++i) {

        PClip clip = 0;

        try {

          AVSValue args[1] = { csps[i].c_str() };
          const char* names[1] = { "pixel_type" };
          clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

        } catch (AvisynthError& err) {

          FAIL(err.msg);

        }

        AVSValue out = 0;

        try {

          AVSValue args[2] = { clip, -1 };
          const char* names[2] = { 0, "mode" };
          out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

        } catch (AvisynthError& err) {

          out = err.msg;

        }

        REQUIRE(out.IsString());
        CHECK(std::string(out.AsString()) ==
              "TurnsTile: " + csps[i] + " only allows modes 0-" +
              modeMaxesMsg[i] + "!");

      }

    }

    SECTION("GreaterThanMax", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26
      int modeMaxes[7] = { 4, 3, 4, 6, 3, 4, 6 };
#else
      int modeMaxes[4] = { 4, 3, 4, 6 };
#endif

      for (int i = 0; i < count; ++i) {

        PClip clip = 0;

        try {

          AVSValue args[1] = { csps[i].c_str() };
          const char* names[1] = { "pixel_type" };
          clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

        } catch (AvisynthError& err) {

          FAIL(err.msg);

        }

        AVSValue out = 0;

        try {

          AVSValue args[2] = { clip, modeMaxes[i] + 1 };
          const char* names[2] = { 0, "mode" };
          out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

        } catch (AvisynthError& err) {

          out = err.msg;

        }

        REQUIRE(out.IsString());
        CHECK(std::string(out.AsString()) ==
              "TurnsTile: " + csps[i] + " only allows modes 0-" +
              modeMaxesMsg[i] + "!");

      }

    }

  }

}



TEST_CASE("Errors/Turnstile/LevelsString", "") {

  PClip clip = 0;

  try {

    clip = env->Invoke("BlankClip", 0).AsClip();

  } catch (AvisynthError& err) {

    FAIL(err.msg);

  }

  AVSValue out;

  try {

    AVSValue args[2] = { clip, "invalid" };
    const char* names[2] = { 0, "levels" };
    out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

  } catch (AvisynthError& err) {

    out = err.msg;

  }

  REQUIRE(out.IsString());
  CHECK(  std::string(out.AsString()) ==
          "TurnsTile: levels must be either \"pc\" or \"tv\"!");

}



TEST_CASE("Errors/Turnstile/LotileRange", "") {

  SECTION("NoTilesheet", "") {

    PClip clip = 0;

    try {

      clip = env->Invoke("BlankClip", 0).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    SECTION("LessThanMin", "") {

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, -1 };
        const char* names[2] = { 0, "lotile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid lotile range is 0-255!");

    }

    SECTION("GreaterThanMax", "") {

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, 256 };
        const char* names[2] = { 0, "lotile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid lotile range is 0-255!");

    }

  }

  SECTION("Tilesheet", "") {

    PClip clip = 0;

    try {

      clip = env->Invoke("BlankClip", 0).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    PClip tilesheet = 0;

    try {

      AVSValue args[2] = { 1024, 1024 };
      const char* names[2] = { "width", "height" };
      tilesheet = env->Invoke("BlankClip", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    SECTION("LessThanMin", "") {

      AVSValue out = 0;

      try {

        AVSValue args[3] = { clip, tilesheet, -1 };
        const char* names[3] = { 0, 0, "lotile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid lotile range is 0-4095!");

    }

    SECTION("GreaterThanMax", "") {

      AVSValue out = 0;

      try {

        AVSValue args[5] = { clip, tilesheet, 16, 16, 4096 };
        const char* names[5] = { 0, 0, "tilew", "tileh", "lotile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 5), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid lotile range is 0-4095!");

    }

  }

}



TEST_CASE("Errors/Turnstile/HitileRange", "") {

  SECTION("NoTilesheet", "") {

    PClip clip = 0;

    try {

      clip = env->Invoke("BlankClip", 0).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    SECTION("LessThanMin", "") {

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, -1 };
        const char* names[2] = { 0, "hitile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid hitile range is 0-255!");

    }

    SECTION("GreaterThanMax", "") {

      AVSValue out = 0;

      try {

        AVSValue args[2] = { clip, 256 };
        const char* names[2] = { 0, "hitile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 2), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid hitile range is 0-255!");

    }

  }

  SECTION("Tilesheet", "") {

    PClip clip = 0;

    try {

      clip = env->Invoke("BlankClip", 0).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    PClip tilesheet = 0;

    try {

      AVSValue args[2] = { 1024, 1024 };
      const char* names[2] = { "width", "height" };
      tilesheet = env->Invoke("BlankClip", AVSValue(args, 2), names).AsClip();

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    SECTION("LessThanMin", "") {

      AVSValue out = 0;

      try {

        AVSValue args[3] = { clip, tilesheet, -1 };
        const char* names[3] = { 0, 0, "hitile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid hitile range is 0-4095!");

    }

    SECTION("GreaterThanMax", "") {

      AVSValue out = 0;

      try {

        AVSValue args[5] = { clip, tilesheet, 16, 16, 4096 };
        const char* names[5] = { 0, 0, "tilew", "tileh", "hitile" };
        out = env->Invoke("TurnsTile", AVSValue(args, 5), names).AsClip();

      } catch (AvisynthError& err) {

        out = err.msg;

      }

      REQUIRE(out.IsString());
      CHECK(std::string(out.AsString()) ==
            "TurnsTile: Valid hitile range is 0-4095!");

    }

  }

}



TEST_CASE("Errors/Turnstile/LotileGreaterThanHitile", "") {

  PClip clip = 0;

  try {

    clip = env->Invoke("BlankClip", 0).AsClip();

  } catch (AvisynthError& err) {

    FAIL(err.msg);

  }

  AVSValue out;

  try {

    AVSValue args[3] = { clip, 255, 0};
    const char* names[3] = { 0, "lotile", "hitile" };
    out = env->Invoke("TurnsTile", AVSValue(args, 3), names).AsClip();

  } catch (AvisynthError& err) {

    out = err.msg;

  }

  REQUIRE(out.IsString());
  CHECK(std::string(out.AsString()) ==
        "TurnsTile: lotile must not be greater than hitile!");

}



TEST_CASE("Errors/Cluter/ColorspaceMismatch", "") {

  PClip clip = 0, palette = 0;

  try {

    AVSValue args[1] = { "RGB32" };
    const char* names[1] = { "pixel_type" };
    clip = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

  } catch (AvisynthError& err) {

    FAIL(err.msg);

  }

  try {

    AVSValue args[1] = { "YV12" };
    const char* names[1] = { "pixel_type" };
    palette = env->Invoke("BlankClip", AVSValue(args, 1), names).AsClip();

  } catch (AvisynthError& err) {

    FAIL(err.msg);

  }

  AVSValue out;

  try {

    AVSValue args[2] = { clip, palette };
    out = env->Invoke("CLUTer", AVSValue(args, 2)).AsClip();

  } catch (AvisynthError& err) {

    out = err.msg;

  }

  REQUIRE(out.IsString());
  CHECK(std::string(out.AsString()) ==
        "CLUTer: clip and palette must share a colorspace!");

}



TEST_CASE("Errors/Cluter/InterlacedHeightMod", "") {

#ifdef TURNSTILE_HOST_AVISYNTH_26

  std::string
    csps[8] = { "RGB32", "RGB24", "YUY2", "YV12",
                "YV24", "YV16", "YV411", "Y8" },
    mods[8] = { "2", "2", "2", "4", "2", "2", "2", "2" };

  int heights[8] = { 479, 479, 479, 478, 479, 479, 479, 479 };

  int count = 8;

#else

  std::string
    csps[4] = { "RGB32", "RGB24", "YUY2", "YV12" },
    mods[4] = { "2", "2", "2", "4" };

  int heights[4] = { 479, 479, 479, 478 };

  int count = 4;

#endif

  for (int i = 0; i < count; ++i) {

    PClip clip = 0, palette = 0;

    try {

      AVSValue args[2] = { heights[i], csps[i].c_str() };
      const char* names[2] = { "height", "pixel_type" };
      clip = env->Invoke("BlankClip", AVSValue(args, 2), names).AsClip();
      palette = clip;

    } catch (AvisynthError& err) {

      FAIL(err.msg);

    }

    AVSValue out = 0;

    try {

      AVSValue args[3] = { clip, palette, true };
      const char* names[3] = { 0, 0, "interlaced" };
      out = env->Invoke("CLUTer", AVSValue(args, 3), names).AsClip();

    } catch (AvisynthError& err) {

      out = err.msg;

    }

    REQUIRE(out.IsString());
    CHECK(std::string(out.AsString()) ==
          "CLUTer: " + csps[i] + " clip height must be mod " + mods[i] +
          " when interlaced=true!");

  }

}
