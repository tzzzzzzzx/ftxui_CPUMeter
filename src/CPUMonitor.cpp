

// standard headers
#include <cstdint>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// local headers
#include "CPUMonitor.hpp"

#define CAL_BETWEEN(c, a, b)                                                   \
    if (c < a) {                                                               \
        c = a;                                                                 \
    } else if (c > b) {                                                        \
        c = b;                                                                 \
    }
#define READ_INTERVAL_MSEC 50

int CPUInfo_sync(CPUInfo *info) {
    FILE *fd;
    char buff[256];
    CPUInfo *cpu_info = info;
    fd = fopen("/proc/stat", "r");
    if (fd == NULL) {
        std::cerr << "ERROR: Failed to open /proc/stat" << std::endl;
        return ENOENT;
    }
    fgets(buff, 256, fd);
    sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_info->name, &(cpu_info->user),
           &(cpu_info->nice), &(cpu_info->system), &(cpu_info->idle),
           &(cpu_info->iowait), &(cpu_info->irq), &(cpu_info->softirq));
    fclose(fd);

    return 0;
}

void CPUInfo_print(CPUInfo *oldinfo, CPUInfo *newinfo) {
    unsigned long od, nd;
    double cpu_use = 0;
    od = (unsigned long)(oldinfo->user + oldinfo->nice + oldinfo->system +
                         oldinfo->idle + oldinfo->iowait + oldinfo->irq +
                         oldinfo->softirq);
    nd = (unsigned long)(newinfo->user + newinfo->nice + newinfo->system +
                         newinfo->idle + newinfo->iowait + newinfo->irq +
                         newinfo->softirq);
    double sum = nd - od;
    double idle = newinfo->idle - oldinfo->idle;
    cpu_use = idle / sum;
    idle = newinfo->user + newinfo->system + newinfo->nice - oldinfo->user -
           oldinfo->system - oldinfo->nice;
    cpu_use = idle / sum;
    printf("cpu_use: %.3f\n", cpu_use);
}

CPUMonitor::CPUMonitor() {
    // this->rapl_power_unit = this->getRaplPowerUnit();
    this->cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
}

int CPUMonitor::syncCPUInfo(int msec) { // msec: read interval
    FILE *fd;
    ssize_t len;
    size_t dym;
    char *buff = NULL;
    for (unsigned int t = 0; t < 2; ++t) {
        fd = fopen("/proc/stat", "r");
        if (fd == NULL)
            return ENOENT;
        // in /proc/stat, the first line is always the general cpuinfo
        // pay attention to the meaning of i here
        len = getline(&buff, &dym, fd);
        if (len != -1) {
            int i = this->cpu_count;
            sscanf(buff, "%s %u %u %u %u %u %u %u %u %u %u",
                   cpu_info[i][t].name, &cpu_info[i][t].user,
                   &cpu_info[i][t].nice, &cpu_info[i][t].system,
                   &cpu_info[i][t].idle, &cpu_info[i][t].iowait,
                   &cpu_info[i][t].irq, &cpu_info[i][t].softirq,
                   &cpu_info[i][t].steal, &cpu_info[i][t].guest,
                   &cpu_info[i][t].guest_nice);
            free(buff);
            buff = NULL;
        }
        for (unsigned int i = 0; i < this->cpu_count; i++) {
            if ((len = getline(&buff, &dym, fd)) != -1) {
                sscanf(buff, "%s %u %u %u %u %u %u %u %u %u %u",
                       cpu_info[i][t].name, &cpu_info[i][t].user,
                       &cpu_info[i][t].nice, &cpu_info[i][t].system,
                       &cpu_info[i][t].idle, &cpu_info[i][t].iowait,
                       &cpu_info[i][t].irq, &cpu_info[i][t].softirq,
                       &cpu_info[i][t].steal, &cpu_info[i][t].guest,
                       &cpu_info[i][t].guest_nice);
                free(buff);
                buff = NULL;
            }
        }
        fclose(fd);
        usleep(msec * 1000);
    }
    return 0;
}

// double CPUMonitor::getCPUPowerTotal(int msec) {
//     int *cpus = (int *)malloc(sizeof(int) * cpu_count);
//     for (int i = 0; i < cpu_count; i++) {
//         cpus[i] = i;
//     }
//     double res = _getCPUPower(cpu_count, cpus, 0, msec);
//     free(cpus);
//     return res;
// }

// double CPUMonitor::_getCPUPower(int n, int *cpus, double energy_units,
//                                 int msec) {
//     int cpu, i;
//     uint64_t data;
//     double *st, *en, *count;
//     st = (double *)malloc(n * sizeof(double));
//     en = (double *)malloc(n * sizeof(double));
//     count = (double *)malloc(n * sizeof(double));
//     for (i = 0; i < n; ++i) {
//         cpu = cpus[i];
//         data = rdmsr(cpu, 0x611);
//         st[i] = (data & 0xffffffff) * energy_units;
//     }
//     usleep(msec * 1000);
//     for (i = 0; i < n; ++i) {
//         cpu = cpus[i];
//         data = rdmsr(cpu, 0x611);
//         en[i] = (data & 0xffffffff) * energy_units;
//         count[i] = 0;
//         if (en[i] < st[i]) {
//             count[i] = (double)(1ll << 32) + en[i] - st[i];
//         } else {
//             count[i] = en[i] - st[i];
//         }
//         count[i] = count[i] / ((double)msec) * 1000.0;
//     }
//     double ret = 0.0;
//     for (i = 0; i < n; ++i) {
//         cpu = cpus[i];
//         printf("get cpu %d power comsumption is: %f\n", cpu, count[i]);
//         ret += count[i];
//     }
//     free(st);
//     free(en);
//     free(count);
//     return ret;
// }

double CPUMonitor::getCPUUsageTotal() {
    double percent = 0;
    percent = _getCPUUsage(this->cpu_count);
    // return percent / this->cpu_count * 100.;
    return percent * 100.;
}
double CPUMonitor::getCPUUsage(int id) { return _getCPUUsage(id) * 100; }

double CPUMonitor::_getCPUUsage(int id) {
    /*
    int ret = syncCPUInfo(msec);
    if (ret < 0) {
        return 0.0;
    }
    */
    double usr2 = cpu_info[id][1].user * 1.;
    double nice2 = cpu_info[id][1].nice * 1.;
    double sys2 = cpu_info[id][1].system * 1.;
    double idle2 = cpu_info[id][1].idle * 1.;
    double wait2 = cpu_info[id][1].iowait * 1.;
    double irq2 = cpu_info[id][1].irq * 1.;
    double softirq2 = cpu_info[id][1].softirq * 1.;
    double steal2 = cpu_info[id][1].steal * 1.;
    double guest2 = cpu_info[id][1].guest * 1.;
    double gn2 = cpu_info[id][1].guest_nice * 1.;
    double usr1 = cpu_info[id][0].user * 1.;
    double nice1 = cpu_info[id][0].nice * 1.;
    double sys1 = cpu_info[id][0].system * 1.;
    double idle1 = cpu_info[id][0].idle * 1.;
    double wait1 = cpu_info[id][0].iowait * 1.;
    double irq1 = cpu_info[id][0].irq * 1.;
    double softirq1 = cpu_info[id][0].softirq * 1.;
    double steal1 = cpu_info[id][0].steal * 1.;
    double guest1 = cpu_info[id][0].guest * 1.;
    double gn1 = cpu_info[id][0].guest_nice * 1.;

    double total = usr2 + nice2 + sys2 + idle2 + wait2 + irq2 + softirq2 +
                   steal2 + guest2 + gn2 - usr1 - nice1 - sys1 - idle1 - wait1 -
                   irq1 - softirq1 - steal1 - guest1 - gn1;

    return (usr2 - usr1) / total + (nice2 - nice1) / total +
           (sys2 - sys1) / total + (wait2 - wait1) / total +
           (irq2 - irq1) / total + (softirq2 - softirq1) / total +
           (steal2 - steal1) / total + (guest2 - guest1) / total +
           (gn2 - gn1) / total;
}

int CPUMonitor::getCPUCount() { return this->cpu_count; }

uint64_t CPUMonitor::rdmsr(int cpu, uint32_t reg) {
    sprintf(buf, "/dev/cpu/%d/msr", cpu);
    int msr_file = open(buf, O_RDONLY);
    if (msr_file < 0) {
        perror("rdmsr: open");
        return msr_file;
    }
    uint64_t data;
    if (pread(msr_file, &data, sizeof(data), reg) != sizeof(data)) {
        fprintf(stderr, "read msr register 0x%x error.\n", reg);
        perror("rdmsr: read msr");
        return -1;
    }
    close(msr_file);
    return data;
}

// RaplPowerUnit CPUMonitor::getRaplPowerUnit(){
//     RaplPowerUnit ret;
//     uint64_t data = rdmsr(0, 0x606);
//     double t = (1 << (data & 0xf));
//     t = 1.0 / t;
//     ret.PU = t;
//     t = (1 << ((data>>8) & 0x1f));
//     ret.ESU = 1.0 / t;
//     t = (1 << ((data>>16) & 0xf));
//     ret.TU = 1.0 / t;
//     return ret;
// }