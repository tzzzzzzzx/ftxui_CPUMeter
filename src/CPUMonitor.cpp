

// standard headers
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>

// local headers
#include "CPUMonitor.hpp"

#define CAL_BETWEEN(c, a, b) \
    if (c < a) {             \
        c = a;               \
    } else if (c > b) {      \
        c = b;               \
    }
#define READ_INTERVAL_MSEC 50

CPUMonitor::CPUMonitor() { this->cpu_count = sysconf(_SC_NPROCESSORS_ONLN); }

/**
 * @brief sync CPUInfo with given interval
 *
 * @param msec interval
 * @return int
 */
int CPUMonitor::syncCPUInfo(int msec) {  // msec: read interval
    FILE *fd;
    ssize_t len;
    size_t dym;
    char *buff = NULL;
    for (unsigned int t = 0; t < 2; ++t) {
        fd = fopen("/proc/stat", "r");
        if (fd == NULL) {
            std::cerr << "ERROR: Failed to open /proc/stat" << std::endl;
            return ENOENT;
        }
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

double CPUMonitor::getCPUUsageTotal() {
    double percent = 0;
    // Note: when id == cpu_count, it shows the total usage according to the
    // structure of /proc/stat.
    percent = _getCPUUsage(this->cpu_count);
    return percent * 100.;
}
double CPUMonitor::getCPUUsage(int id) { return _getCPUUsage(id) * 100; }

double CPUMonitor::_getCPUUsage(int id) {
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
