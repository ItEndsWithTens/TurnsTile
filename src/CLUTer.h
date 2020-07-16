#ifndef TURNSTILE_SRC_CLUTER_H_INCLUDED
#define TURNSTILE_SRC_CLUTER_H_INCLUDED



#include <vector>

#include "interface.h"



class CLUTer : public GenericVideoFilter
{

public:

  CLUTer(PClip _child, PClip _palette,
         int _pltFrame, bool _interlaced,
         IScriptEnvironment* env);

  ~CLUTer();

  PVideoFrame __stdcall GetFrame(
    int n, IScriptEnvironment* env);

  int __stdcall SetCacheHints(int cachehints, int frame_range);

private:

  std::vector<unsigned char> vecYR, vecUG, vecVB;

  int spp, lumaW, lumaH;

  bool PLANAR, YUYV, BGRA, BGR;

  void buildPalettePacked(
    const unsigned char* pltp, int width, int height,
    const int PLT_PITCH_SAMPLES);

  void processFramePacked(
    const unsigned char* srcp, unsigned char* dstp,
    int width, int height,
    const int SRC_PITCH_SAMPLES, const int DST_PITCH_SAMPLES);

  void buildPalettePlanar(
    const unsigned char* pltY,
    const unsigned char* pltU,
    const unsigned char* pltV,
    int widthU, int heightU,
    const int PLT_PITCH_SAMPLES_Y, const int PLT_PITCH_SAMPLES_U);

  void processFramePlanar(
    const unsigned char* srcY,
    const unsigned char* srcU,
    const unsigned char* srcV,
    unsigned char* dstY,
    unsigned char* dstU,
    unsigned char* dstV,
    const int SRC_WIDTH_U, const int SRC_HEIGHT_U,
    const int SRC_PITCH_SAMPLES_Y, const int SRC_PITCH_SAMPLES_U,
    const int DST_PITCH_SAMPLES_Y, const int DST_PITCH_SAMPLES_U);

  void fillComponentVectors(std::vector<int>* pltMain);

};



#endif // TURNSTILE_SRC_CLUTER_H_INCLUDED
