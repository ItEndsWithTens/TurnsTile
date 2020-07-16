#ifndef TURNSTILE_TEST_SRC_UTIL_COMMON_H_INCLUDED
#define TURNSTILE_TEST_SRC_UTIL_COMMON_H_INCLUDED



#include <string>
#include <vector>



struct plane
{

  const unsigned char* ptr;
  int row_size, pitch, height;

};



std::string GetFrameHash(std::vector<plane> planes);



std::string ReadRefData(std::string filename);



std::string SplitError(std::string err);



int WriteRefData(std::string hash, std::string filename);



void CompareData(std::string data, std::string filename);



#endif // TURNSTILE_TEST_SRC_UTIL_COMMON_H_INCLUDED
