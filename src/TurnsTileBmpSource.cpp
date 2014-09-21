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



// I added this custom source filter to allow reading my reference EBMP files in
// Avxsynth, which at the moment doesn't have any suitable sources. FFMS2 is all
// that's available, and it doesn't support the format. Research didn't reveal
// any other file type that allowed storage of a frame in its native colorspace,
// so this seemed like the least bad option.
//
// Ugly as it may be to have made this filter part of the plugin itself, instead
// of putting the code somewhere in the test executable, I wanted to stick to my
// goal of allowing all test scripts to stand alone. With TurnsTileBmpSource as
// part of the test runner, the scripts wouldn't load if accessed from another
// program, for example VirtualDub or AvsPmod.



#include "TurnsTileBmpSource.h"

#include <cmath>
#include <fstream>
#include <string>
#include <vector>

#include "interface.h"



#ifdef TURNSTILE_HOST_AVXSYNTH

#define IScriptEnvironment avxsynth::IScriptEnvironment
#define VideoInfo avxsynth::VideoInfo
#define SAMPLE_INT16 avxsynth::SAMPLE_INT16
#define PVideoFrame avxsynth::PVideoFrame
#define __int64 avxsynth::__int64
#define AVSValue avxsynth::AVSValue
#define PLANAR_Y avxsynth::PLANAR_Y
#define PLANAR_U avxsynth::PLANAR_U
#define PLANAR_V avxsynth::PLANAR_V

#endif



void DecodeBmpPacked(
  std::vector<unsigned char>& buf, unsigned int ofsData, unsigned char* dst,
  const int PITCH, const int HEIGHT, const int ROW_SIZE, const int PITCH_BUF)
{

  for (int i = 0; i < HEIGHT; ++i)
    for (int j = 0; j < ROW_SIZE; ++j)
      *(dst + (PITCH * i) + j) = buf[ofsData + (PITCH_BUF * i) + j];

}



void DecodeBmpPlanar(
  std::vector<unsigned char>& buf, unsigned int ofsData,
  unsigned char* dstY, unsigned char* dstU, unsigned char* dstV,
  const int PITCH_Y, const int HEIGHT_Y, const int ROW_SIZE_Y,
  const int PITCH_U, const int HEIGHT_U, const int ROW_SIZE_U)
{

  const int OFS_BUF_Y = ofsData,
            OFS_BUF_U = OFS_BUF_Y + (ROW_SIZE_Y * HEIGHT_Y),
            OFS_BUF_V = OFS_BUF_U + (ROW_SIZE_U * HEIGHT_U);

  for (int i = 0; i < HEIGHT_Y; ++i)
    for (int j = 0; j < ROW_SIZE_Y; ++j)
      *(dstY + (PITCH_Y * i) + j) = buf[OFS_BUF_Y + (i * ROW_SIZE_Y) + j];

  for (int i = 0; i < HEIGHT_U; ++i)
    for (int j = 0; j < ROW_SIZE_U; ++j)
      *(dstU + (PITCH_U * i) + j) = buf[OFS_BUF_U + (i * ROW_SIZE_U) + j];

  for (int i = 0; i < HEIGHT_U; ++i)
    for (int j = 0; j < ROW_SIZE_U; ++j)
      *(dstV + (PITCH_U * i) + j) = buf[OFS_BUF_V + (i * ROW_SIZE_U) + j];

}



std::string OpenFile(std::string& filename, std::vector<unsigned char>& buf)
{

  std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
  if (!file.is_open())
    return "TurnsTileBMPSource: Could not open " + filename + "!";

  file.seekg(0, std::ios::end);
  int len = static_cast<int>(file.tellg());
  file.seekg(0, std::ios::beg);
  buf.resize(len);
  file.read(reinterpret_cast<char*>(&buf[0]), len);
  file.close();

  if (!(buf[0] == 'B' && buf[1] == 'M'))
    return "TurnsTileBMPSource: Input must be BMP or EBMP!";

  return "";

}



TurnsTileBmpSource::TurnsTileBmpSource(
  std::string filename, std::string pixel_type, IScriptEnvironment* env)
{
  
  std::vector<unsigned char> buf;

  std::string err = OpenFile(filename, buf);
  if (err != "")
    env->ThrowError(err.c_str());

  vi.audio_samples_per_second = 0;
  vi.fps_denominator = 1;
  vi.fps_numerator = 24;
  vi.num_frames = 240;

  unsigned int
    ofsInfoHdr = 14,
    ofsData = *(reinterpret_cast<unsigned int*>(&buf[10]));

  vi.width = *(reinterpret_cast<int*>(&buf[ofsInfoHdr + 4]));
  vi.height = *(reinterpret_cast<int*>(&buf[ofsInfoHdr + 8]));

  short int
    planes = *(reinterpret_cast<short int*>(&buf[ofsInfoHdr + 12])),
    bits = *(reinterpret_cast<short int*>(&buf[ofsInfoHdr + 14]));

  unsigned int
    compression = *(reinterpret_cast<short int*>(&buf[ofsInfoHdr + 16]));

  if (compression)
    env->ThrowError("TurnsTileBMPSource: Cannot load compressed BMPs!");

  if (planes == 1) {
    if (bits == 32)
      vi.pixel_type = VideoInfo::CS_BGR32;
    else if (bits == 24)
      vi.pixel_type = VideoInfo::CS_BGR24;
    else if (bits == 16)
      vi.pixel_type = VideoInfo::CS_YUY2;
    else
      env->ThrowError(
        "TurnsTileBMPSource: Cannot load 1 plane, %d bit BMPs!", bits);
  } else if (planes == 3) {
    if (bits == 12)
      vi.pixel_type = VideoInfo::CS_YV12;
    else
      env->ThrowError(
        "TurnsTileBMPSource: Cannot load 3 plane, %d bit BMPs!", bits);
  } else {
    env->ThrowError("TurnsTileBMPSource: Cannot load %d plane BMPs!", planes);
  }

  frm = env->NewVideoFrame(vi);

  unsigned char
    * dstY = frm->GetWritePtr(PLANAR_Y),
    * dstU = frm->GetWritePtr(PLANAR_U),
    * dstV = frm->GetWritePtr(PLANAR_V);

  const int
    PITCH_Y = frm->GetPitch(PLANAR_Y),
    PITCH_U = frm->GetPitch(PLANAR_U),
    HEIGHT_Y = frm->GetHeight(PLANAR_Y),
    HEIGHT_U = frm->GetHeight(PLANAR_U),
    ROW_SIZE_Y = frm->GetRowSize(PLANAR_Y),
    ROW_SIZE_U = frm->GetRowSize(PLANAR_U),
    PITCH_BUF = static_cast<int>(std::ceil(((bits / 8) * vi.width) / 4.0)) * 4;

  if (planes == 1)
    DecodeBmpPacked(
      buf, ofsData, dstY,
      PITCH_Y, HEIGHT_Y, ROW_SIZE_Y, PITCH_BUF);
  else
    DecodeBmpPlanar(
      buf, ofsData,
      dstY, dstU, dstV,
      PITCH_Y, HEIGHT_Y, ROW_SIZE_Y,
      PITCH_U, HEIGHT_U, ROW_SIZE_U);  

}



AVSValue __cdecl Create_TurnsTileBmpSource(
  AVSValue args, void* user_data, IScriptEnvironment* env)
{

  std::string filename = args[0].AsString(""),
              pixel_type = args[1].AsString("RGB32");

  return new TurnsTileBmpSource(filename, pixel_type, env);

}
