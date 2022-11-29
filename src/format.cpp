#include "format.h"

#include <string>

using std::string;

template <typename T>
T Division(T a, T b) {
  return a / b;
}

string Format::ElapsedTime(long seconds) {
  char tempBuffer[100];
  sprintf(tempBuffer, "%02ld:%02ld:%02ld", Division<long>(seconds, 3600),      //Using template to divide numbers
          Division<long>((seconds % 3600), 60), seconds % 60);
  string TimeFormat = tempBuffer;

  return TimeFormat;
}