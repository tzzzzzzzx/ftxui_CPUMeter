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

    double _getCPUPower(int n, int *cpus, double energy_units, int msec);

    double _getCPUUsage(int id);

} CPUMonitor;

#endif