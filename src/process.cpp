#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  long TotalJiffies_sec = long(LinuxParser::ActiveJiffies(Pid())) / sysconf(_SC_CLK_TCK);
  float cpu_usage = float(TotalJiffies_sec) / float(UpTime());

  #ifdef DEBUGG
  std::ofstream MyFile("DEBUG.txt", std::ios::app);
  MyFile << "jiffes_sec: " << float(TotalJiffies_sec) << "\n"; 
  MyFile << "uptime_sec: " << float(UpTime()) << "\n"; 
  MyFile << "cpu_usage: " << cpu_usage << "\n"; 
  MyFile.close(); 
  #endif

  return cpu_usage;
 }

// TODO: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(Pid()); }

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Pid()); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Pid()); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime() - LinuxParser::UpTime(Pid()); } // Time system started (running)- time process started (stopped).

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process & a) { return stol(a.Ram()) < stol(Ram()); } 