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



#include <fstream>
#include <ios>
#include <string>
#include <sstream>

#include "../include/catch/catch.hpp"



extern bool writeRefData;



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
    if (dataRef == "")
      FAIL("Could not read " + filename + "!");
    else
      CHECK(dataCur == dataRef);

  }

}
