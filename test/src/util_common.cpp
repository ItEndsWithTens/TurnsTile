#include <fstream>
#include <ios>
#include <string>
#include <sstream>
#include <vector>

#include "../include/catch/catch.hpp"
#include "../include/md5/md5.h"

#include "util_common.h"



extern bool writeRefData;



std::string GetFrameHash(std::vector<plane> planes)
{

  md5_state_t state;
  md5_byte_t digest[16];

  md5_init(&state);
  for (std::vector<plane>::iterator p = planes.begin(); p != planes.end(); ++p)
    for (int i = 0; i < p->height; ++i)
      md5_append(&state, (md5_byte_t*)(p->ptr + (p->pitch * i)), p->row_size);
  md5_finish(&state, digest);

  std::string hash;

  for (int i = 0; i < 16; ++i) {
    const char* lut = "0123456789abcdef";
    hash.push_back(lut[digest[i] >> 4]);
    hash.push_back(lut[digest[i] & 15]);
  }

  return hash;

}



std::string ReadRefData(std::string filename)
{

  std::ifstream fileIn(filename.c_str(), std::ios::in);

  if (!fileIn) {

    return "";

  } else {

    std::stringstream stream;
    stream << fileIn.rdbuf();
    std::string ref(stream.str());
    fileIn.close();

    return ref;

  }

}



std::string SplitError(std::string err)
{

  return err.substr(0, err.find('\n'));

}



int WriteRefData(std::string data, std::string filename)
{

  std::ofstream fileOut(filename.c_str(), std::ios::out);

  if (!fileOut) {

    return -1;

  } else {

    fileOut.write(data.c_str(), data.length());
    fileOut.close();

    return 0;

  }

}



void CompareData(std::string dataCur, std::string filename)
{

  if (writeRefData) {

    if (WriteRefData(dataCur, filename) != 0)
      FAIL("Could not write " + filename + "!");

  } else {

    std::string dataRef = ReadRefData(filename);
    if (dataRef == "") {

      FAIL("Could not read " + filename + "!");

    } else {

      INFO("Current result differs from that stored in " + filename);
      CHECK(dataCur == dataRef);

    }

  }

}
