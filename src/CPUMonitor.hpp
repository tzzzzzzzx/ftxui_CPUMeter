#ifndef _CPUMONITOR_H
#define _CPUMONITOR_H

#include <stdint.h>
#include <string.h>

typedef struct CPUInfo {
    char name[256];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
    unsigned int steal;
    unsigned int guest;
    unsigned int guest_nice;
    CPUInfo() { memset(name, '\0', 256); }
} CPUInfo;

// typedef struct RaplPowerUnit{
//     //设置成double以满足Intel官方手册的64位要求。详见14-32节MSR寄存器
//     double PU;      //power unit
//     double ESU;     //energy status unit
//     double TU;      //time unit
// }RaplPowerUnit;

typedef class CPUMonitor {
  public:
    CPUMonitor();
    double getCPUPowerTotal(int msec);
    double getCPUUsageTotal();
    double getCPUUsage(int id);
    int syncCPUInfo(int msec);
    int getCPUCount();

  private:
    char buf[1024];

    int cpu_count;
    CPUInfo cpu_info[128][2];
    CPUInfo general_cpu_info[2];

    //   RaplPowerUnit rapl_power_unit;

    double _getCPUPower(int n, int *cpus, double energy_units, int msec);

    double _getCPUUsage(int id);

    uint64_t rdmsr(int cpu, uint32_t reg);

    // RaplPowerUnit getRaplPowerUnit();

} CPUMonitor;

int CPUInfo_sync(CPUInfo *info);
void CPUInfo_print(CPUInfo *oldinfo, CPUInfo *newinfo);

#endif