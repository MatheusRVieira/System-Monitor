#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

#define DEBUG 0


// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string token, str, MemTotal_str = "0", MemFree_str = "0";
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> str;
      if (token == "MemTotal:") MemTotal_str = str;
      if (token == "MemFree:") MemFree_str = str;
    }
  }

  #ifdef DEBUGG
  std::ofstream MyFile("DEBUG.txt");
  MyFile << std::stof(MemTotal_str) << "\n"; 
  MyFile << std::stof(MemFree_str) << "\n";
  MyFile << 100 * (std::stof(MemTotal_str) - std::stof(MemFree_str)) / std::stof(MemTotal_str) << "\n";
  MyFile.close(); 
  #endif
  
  // MemoryUtilization = (MemTotal - MemFree) / MemTotal 
  return (std::stof(MemTotal_str) - std::stof(MemFree_str)) / std::stof(MemTotal_str);
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string token, totalProcesses_str;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);  
  if(stream.is_open()){
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> totalProcesses_str;
      if(token == "processes") break;
    }
  }
  return std::stoi(totalProcesses_str); 
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string token, runningProcesses_str;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);  
  if(stream.is_open()){
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> runningProcesses_str;
      if(token == "procs_running") break;
    }
  }
  return std::stoi(runningProcesses_str); 
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime_str;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime_str;
  }

  #ifdef DEBUGG
  std::ofstream MyFile("DEBUG.txt");
  MyFile << std::stof(uptime_str) << "\n"; 
  MyFile.close(); 
  #endif

  return std::stol(uptime_str); 
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {  
  string uptime_clk;
  long uptime_sec;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(stream.is_open()){
    std::getline(stream,line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; i++)
      linestream >> uptime_clk;
    
    uptime_sec = stol(uptime_clk) / sysconf(_SC_CLK_TCK); // The time the process started after system boot.
  }

  #ifdef DEBUGG
  std::ofstream MyFile("DEBUG.txt", std::ios::app);
  MyFile << uptime_sec << "\n"; 
  MyFile << uptime_clk << "\n"; 
  MyFile << uptime_sec << "\n"; 
  MyFile.close(); 
  #endif

  return uptime_sec;

}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
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

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string comm_str;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(stream.is_open()){
    std::getline(stream,line);
    std::replace(line.begin(), line.end(),'\0',' '); //Command line with /0 throught the line
  }

  #ifdef DEBUG
  std::ofstream MyFile("DEBUG.txt", std::ios::app);
  MyFile << line << "\n"; 
  MyFile.close(); 
  #endif

  return line;
  
 }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {

  string token, uid_str;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);  
  
  if(stream.is_open()){

    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> uid_str;
      if(token == "Uid:"){
        return uid_str; 
        break; 
      }
    }
  }

  return uid_str; 

}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
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

      if (x == "x" && uid == Uid_func)
        return username;    
      }
    }

  #ifdef DEBUGG
  std::ofstream MyFile("DEBUG.txt");
  MyFile << "pid: " << pid << "\n"; 
  MyFile << "uid: " << uid << "\n"; 
  MyFile << "user: " << username << "\n"; 
  MyFile.close(); 
  #endif

  return username;
}
