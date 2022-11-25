#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
  char tempBuffer[100];
  sprintf(tempBuffer, "%02ld:%02ld:%02ld", seconds / 3600, (seconds % 3600) / 60, seconds % 60);
  string TimeFormat = tempBuffer;
  return TimeFormat; 
}