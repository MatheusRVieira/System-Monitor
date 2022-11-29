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
          return value;
        }
      }
    }
  }
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
  return kernel;
}

float LinuxParser::MemoryUtilization() {
  string token, str, MemTotal_str = "0", MemFree_str = "0";
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> str;
      if (token == "MemTotal:") MemTotal_str = str;
      if (token == "MemFree:") MemFree_str = str;
    }
  }

  // MemoryUtilization = (MemTotal - MemFree) / MemTotal
  return (std::stof(MemTotal_str) - std::stof(MemFree_str)) / std::stof(MemTotal_str);
}

int LinuxParser::TotalProcesses() {
  string token, totalProcesses_str;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> totalProcesses_str;
      if (token == "processes") return std::stoi(totalProcesses_str);
    }
  }
  return std::stoi(totalProcesses_str);
}

int LinuxParser::RunningProcesses() {
  string token, runningProcesses_str;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> runningProcesses_str;
      if (token == "procs_running") return std::stoi(runningProcesses_str);
    }
  }
  return std::stoi(runningProcesses_str);
}

long LinuxParser::UpTime() {
  string uptime_str;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime_str;
  }

  return std::stol(uptime_str);
}

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
        sysconf(_SC_CLK_TCK);  // The time the process started after system boot.
  }

  return uptime_sec;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
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

  return cpu_columns;
}

string LinuxParser::Command(int pid) {
  string comm_str;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::replace(line.begin(), line.end(), '\0',' ');  // Command line with /0 throught the line
  }

  return line;
}

string LinuxParser::Ram(int pid) {
  string token;
  long VmSize_l;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> VmSize_l;
      if (token == "VmSize:") {
        VmSize_l /= 1000;

        return to_string(VmSize_l);
      }
    }
  }

  return to_string(VmSize_l);
}

string LinuxParser::Uid(int pid) {
  string token, uid_str;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> uid_str;
      if (token == "Uid:") {
        return uid_str;
      }
    }
  }

  return uid_str;
}

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

      if (x == "x" && uid == Uid_func) return username;
    }
  }

  return username;
}
