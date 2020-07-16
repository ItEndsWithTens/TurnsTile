#ifndef TURNSTILE_SRC_TURNSTILETESTSOURCE_H_INCLUDED
#define TURNSTILE_SRC_TURNSTILETESTSOURCE_H_INCLUDED



#include <cstdint>
#include <string>
#include <vector>

#include "interface.h"



enum 
{
  FILETYPE_UNKNOWN,
  FILETYPE_BMP,
  FILETYPE_EBMP,
  FILETYPE_PNG
};



class TurnsTileTestSource : public IClip
{

public:

  TurnsTileTestSource(
    std::string filename, std::string pixel_type, IScriptEnvironment* env);
  ~TurnsTileTestSource() {}

  void __stdcall GetAudio(
    void* buf, std::int64_t start, std::int64_t count, IScriptEnvironment* env) {}
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return frm; }
  bool __stdcall GetParity(int n) { return false; }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  int __stdcall SetCacheHints(int cachehints, int frame_range);

private:

  PVideoFrame frm;
  VideoInfo vi;

  void DecodeBmp(std::vector<unsigned char>& raw, IScriptEnvironment* env);
  void DecodeBmpPacked(
    std::vector<unsigned char>& buf, unsigned int ofsData, unsigned char* dst,
    const int PITCH, const int HEIGHT, const int ROW_SIZE, const int PITCH_BUF);
  void DecodeBmpPlanar(
    std::vector<unsigned char>& buf, unsigned int ofsData,
    unsigned char* dstY, unsigned char* dstU, unsigned char* dstV,
    const int PITCH_Y, const int HEIGHT_Y, const int ROW_SIZE_Y,
    const int PITCH_U, const int HEIGHT_U, const int ROW_SIZE_U);
  void DecodePng(std::vector<unsigned char>& raw, std::string pixel_type, IScriptEnvironment* env);
  int OpenFile(std::string& filename, std::vector<unsigned char>& buf, IScriptEnvironment* env);
  

};



AVSValue __cdecl Create_TurnsTileTestSource(
  AVSValue args, void* user_data, IScriptEnvironment* env);

#endif // TURNSTILE_SRC_TURNSTILETESTSOURCE_H_INCLUDED
