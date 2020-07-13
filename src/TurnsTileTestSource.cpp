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



#include "TurnsTileTestSource.h"

#include <cmath>
#include <fstream>
#include <string>
#include <vector>

#include "lodepng/lodepng.h"

#include "interface.h"



TurnsTileTestSource::TurnsTileTestSource(
  std::string filename, std::string pixel_type, IScriptEnvironment* env)
{

  std::vector<unsigned char> raw;

  int type = OpenFile(filename, raw, env);

  vi.audio_samples_per_second = 0;
  vi.fps_denominator = 1;
  vi.fps_numerator = 24;
  vi.num_frames = 240;

  if (type == FILETYPE_PNG)
    DecodePng(raw, pixel_type, env);
  else
    DecodeBmp(raw, env);

}



void TurnsTileTestSource::DecodeBmp(std::vector<unsigned char>& raw, IScriptEnvironment* env)
{

  unsigned int
    ofsInfoHdr = 14,
    ofsData = *(reinterpret_cast<unsigned int*>(&raw[10]));

  vi.width = *(reinterpret_cast<int*>(&raw[ofsInfoHdr + 4]));
  vi.height = *(reinterpret_cast<int*>(&raw[ofsInfoHdr + 8]));

  short int
    planes = *(reinterpret_cast<short int*>(&raw[ofsInfoHdr + 12])),
    bits = *(reinterpret_cast<short int*>(&raw[ofsInfoHdr + 14]));

  unsigned int
    compression = *(reinterpret_cast<short int*>(&raw[ofsInfoHdr + 16]));

  if (compression)
    env->ThrowError("TurnsTileTestSource: Cannot load compressed BMPs!");

  if (planes == 1) {
    if (bits == 32)
      vi.pixel_type = VideoInfo::CS_BGR32;
    else if (bits == 24)
      vi.pixel_type = VideoInfo::CS_BGR24;
    else if (bits == 16)
      vi.pixel_type = VideoInfo::CS_YUY2;
    else
      env->ThrowError(
        "TurnsTileTestSource: Cannot load 1 plane, %d bit BMPs!", bits);
  } else if (planes == 3) {
    if (bits == 12)
      vi.pixel_type = VideoInfo::CS_YV12;
    else
      env->ThrowError(
        "TurnsTileTestSource: Cannot load 3 plane, %d bit BMPs!", bits);
  } else {
    env->ThrowError("TurnsTileTestSource: Cannot load %d plane BMPs!", planes);
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
      raw, ofsData, dstY,
      PITCH_Y, HEIGHT_Y, ROW_SIZE_Y, PITCH_BUF);
  else
    DecodeBmpPlanar(
      raw, ofsData,
      dstY, dstU, dstV,
      PITCH_Y, HEIGHT_Y, ROW_SIZE_Y,
      PITCH_U, HEIGHT_U, ROW_SIZE_U);

}



void TurnsTileTestSource::DecodeBmpPacked(
  std::vector<unsigned char>& buf, unsigned int ofsData, unsigned char* dst,
  const int PITCH, const int HEIGHT, const int ROW_SIZE, const int PITCH_BUF)
{

  for (int i = 0; i < HEIGHT; ++i)
    for (int j = 0; j < ROW_SIZE; ++j)
      *(dst + (PITCH * i) + j) = buf[ofsData + (PITCH_BUF * i) + j];

}



void TurnsTileTestSource::DecodeBmpPlanar(
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



void TurnsTileTestSource::DecodePng(std::vector<unsigned char>& raw, std::string pixel_type, IScriptEnvironment* env)
{

  std::vector<unsigned char> buf;
  unsigned int width, height;

  unsigned int error = 0;
  if (pixel_type == "RGB24") {

    vi.pixel_type = VideoInfo::CS_BGR24;

    lodepng::State state;
    state.info_raw.colortype = LCT_RGB;

    error = lodepng::decode(buf, width, height, state, raw);

  } else {

    vi.pixel_type = VideoInfo::CS_BGR32;

    lodepng::State state;
    state.info_raw.colortype = LCT_RGBA;

    error = lodepng::decode(buf, width, height, state, raw);

  }

  if (error)
    env->ThrowError("TurnsTileTestSource: Couldn't decode PNG! LodePNG error: %s", lodepng_error_text(error));

  vi.width = width;
  vi.height = height;

  frm = env->NewVideoFrame(vi);

  unsigned char* dst = frm->GetWritePtr();

  const int
    PITCH = frm->GetPitch(),
    HEIGHT = frm->GetHeight(),
    ROW_SIZE = frm->GetRowSize(),
    PITCH_BUF = vi.BytesFromPixels(1) * vi.width;

  for (int i = 0; i < HEIGHT; ++i) {

    int ofsFrm = PITCH * i;
    int ofsBuf = PITCH_BUF * (HEIGHT - 1 - i);

    for (int j = 0; j < ROW_SIZE; j += vi.BytesFromPixels(1)) {

      // LodePNG defaults to providing a buffer of RGBA data, whereas the RGB32
      // pixel type in Avisynth is BGRA. The number of bytes per pixel remains
      // the same, but the red and blue components need to change places.
      *(dst + ofsFrm + j + 0) = buf[ofsBuf + j + 2];
      *(dst + ofsFrm + j + 1) = buf[ofsBuf + j + 1];
      *(dst + ofsFrm + j + 2) = buf[ofsBuf + j + 0];
      if (vi.pixel_type == VideoInfo::CS_BGR32)
        *(dst + ofsFrm + j + 3) = buf[ofsBuf + j + 3];

    }

  }

}



int TurnsTileTestSource::OpenFile(std::string& filename, std::vector<unsigned char>& buf, IScriptEnvironment* env)
{

  std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
  if (!file.is_open())
    env->ThrowError("TurnsTileTestSource: Couldn't open %s!", filename.c_str());

  file.seekg(0, std::ios::end);
  int len = static_cast<int>(file.tellg());
  file.seekg(0, std::ios::beg);
  buf.resize(len);
  file.read(reinterpret_cast<char*>(&buf[0]), len);
  file.close();

  int type = FILETYPE_UNKNOWN;

  if (buf[0] == 'B' && buf[1] == 'M') {
    if (filename.substr(filename.find_last_of('.') + 1) == ".ebmp")
      type = FILETYPE_EBMP;
    else
      type = FILETYPE_BMP;
  } else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4e && buf[3] == 0x47 &&
             buf[4] == 0x0d && buf[5] == 0x0a && buf[6] == 0x1a && buf[7] == 0x0a) {
    type = FILETYPE_PNG;
  }

  if (type == FILETYPE_UNKNOWN)
    env->ThrowError("TurnsTileTestSource: Input must be BMP, EBMP, or PNG!");

  return type;

}



int __stdcall TurnsTileTestSource::SetCacheHints(int cachehints, int frame_range)
{

  int hints = 0;

  if (cachehints == CACHE_GETCHILD_ACCESS_COST) {
    hints = CACHE_ACCESS_RAND;
  }
  else if (cachehints == CACHE_GETCHILD_COST) {
    hints = CACHE_COST_LOW;
  }
  else if (cachehints == CACHE_GETCHILD_THREAD_MODE) {
    hints = CACHE_THREAD_SAFE;
  }
  else if (cachehints == CACHE_GET_MTMODE) {
    hints = MT_NICE_FILTER;
  }

  return hints;

}
