#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  float res = float(LinuxParser::ActiveJiffies()) / float(LinuxParser::Jiffies());
        #ifdef DEBUGG
        std::ofstream MyFile("DEBUG.txt", std::ios::app);
        MyFile << float(LinuxParser::ActiveJiffies()) << "\n"; 
        MyFile << float(LinuxParser::Jiffies()) << "\n"; 
        MyFile << res << "\n"; 
        MyFile.close(); 
        #endif  
  return res;
}