#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

template <typename T>
T getSecondToken(std::string const &KeyFilter, std::string const &filename) {
  std::string line, key;
  T value;
  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == KeyFilter) {
          stream.close();
          return value;
        }
      }
    }
  }
  stream.close();
  return value;
};

template <typename T>
T getValueOfFile(std::string const &filename) {
  std::string line;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  }
  stream.close();
  return value;
};

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          filestream.close();
          return value;
        }
      }
    }
  }
  filestream.close();
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();
  return kernel;
}

float LinuxParser::MemoryUtilization() {
  float Total = getSecondToken<float>(filterMemTotal, kMeminfoFilename);
  float Free = getSecondToken<float>(filterMemFree, kMeminfoFilename);
  return (Total - Free) / Total;
}

int LinuxParser::TotalProcesses() { return getSecondToken<int>(filterProcesses, kStatFilename); }

int LinuxParser::RunningProcesses() { return getSecondToken<int>(filterProcs_running, kStatFilename); }

long LinuxParser::UpTime() { return getValueOfFile<long>(kUptimeFilename); }

long LinuxParser::UpTime(int pid) {
  string uptime_clk;
  long uptime_sec;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; i++) linestream >> uptime_clk;

    uptime_sec =
        stol(uptime_clk) /
        sysconf(
            _SC_CLK_TCK);  // The time the process started after system boot.
  }
  stream.close();
  return uptime_sec;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR *directory = opendir(kProcDirectory.c_str());
  struct dirent *file;
  while ((file = readdir(directory)) != nullptr) {
    if (file->d_type == DT_DIR) {
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

long LinuxParser::ActiveJiffies(int pid) {
  long TotalJiffies = 0;
  string token;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 1; i <= 17; i++) {
      linestream >> token;
      if (i >= 14 && i <= 17)  // Including the time for children processes
        TotalJiffies += stol(token);
    }
  }
  stream.close();
  return TotalJiffies;
}

long LinuxParser::ActiveJiffies() {
  vector<string> CPU_jiffies = CpuUtilization();
  return stol(CPU_jiffies[CPUStates::kUser_]) +
         stol(CPU_jiffies[CPUStates::kNice_]) +
         stol(CPU_jiffies[CPUStates::kSystem_]) +
         stol(CPU_jiffies[CPUStates::kIRQ_]) +
         stol(CPU_jiffies[CPUStates::kSoftIRQ_]) +
         stol(CPU_jiffies[CPUStates::kSteal_]);
}

long LinuxParser::IdleJiffies() {
  vector<string> CPU_jiffies = CpuUtilization();
  return stol(CPU_jiffies[CPUStates::kIdle_]) +
         stol(CPU_jiffies[CPUStates::kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpu_columns;
  string token;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> token;  // Get "cpu" token
    for (int i = 0; i < 10; i++) {
      linestream >> token;
      cpu_columns.push_back(token);
    }
  }
  stream.close();
  return cpu_columns;
}

string LinuxParser::Command(int pid) {
  string command = string(getValueOfFile<std::string>(std::to_string(pid) + kCmdlineFilename));
  if(command.length() > 50){
    command.resize(50);
    command += "...";
  }
  return command;
}

string LinuxParser::Ram(int pid) {  //  Used VmRSS instead of VmSize
  return to_string(
      getSecondToken<long>(filterVmRSS, std::to_string(pid) + kStatusFilename) /
      1024); // Convert to MB
}

string LinuxParser::Uid(int pid) { return getSecondToken<string>(filterUid, std::to_string(pid) + kStatusFilename); }

string LinuxParser::User(int pid) {
  string line;
  string Uid_func = LinuxParser::Uid(pid);

  string username, x, uid;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> username >> x >> uid;

      if (x == "x" && uid == Uid_func) {
        filestream.close();
        return username;
      }
    }
  }
  filestream.close();
  return username;
}
