#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() {
  float res = float(LinuxParser::ActiveJiffies()) / float(LinuxParser::Jiffies());
  return res;
}