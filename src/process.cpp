#include "process.h"
#include "linux_parser.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return pid_; }

float Process::CpuUtilization() {
  long TotalJiffies_sec = long(LinuxParser::ActiveJiffies(Pid())) / sysconf(_SC_CLK_TCK);
  float cpu_usage = float(TotalJiffies_sec) / float(UpTime());

  return cpu_usage;
}

string Process::Command() { return LinuxParser::Command(Pid()); }

string Process::Ram() { return LinuxParser::Ram(Pid()); }

string Process::User() { return LinuxParser::User(Pid()); }

long int Process::UpTime() {
  return LinuxParser::UpTime() - LinuxParser::UpTime(Pid());
}  // Time system started (running)- time process started (stopped).

bool Process::operator<(Process& a) {
  return stol(a.Ram()) < stol(Ram());
}  // Used to sort processes by RAM usage