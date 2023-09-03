#include <Pdh.h>
#include <Windows.h>
#include <profileapi.h>
#include <winnt.h>

#include <iostream>
#include <vector>

#pragma comment(lib, "pdh.lib")

extern const char *CPU_QUERIES[128];

struct CPUInfo {
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE valueOld, valueNew;
};

class CPUMonitor {
   public:
    CPUMonitor() {
        // Get the total number of processors in the system
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        m_cpuCount = systemInfo.dwNumberOfProcessors;

        // Init query for each CPU
        for (int i = 0; i < m_cpuCount; i++) {
            CPUInfo info;
            PdhOpenQuery(NULL, NULL, &info.query);
            PdhAddEnglishCounterA(info.query, CPU_QUERIES[i], NULL,
                                  &info.counter);
            PdhCollectQueryData(info.query);
            m_cpuInfos.push_back(info);
        }
    }
    double getCPUUsageTotal() {
        double avg = 0.;
        for (int i = 0; i < m_cpuCount; i++) {
            avg += getCPUUsage(i);
        }
        return avg / (1. * m_cpuCount);
    }
    double getCPUUsage(int id) {
        // PDH
        if (id < 0 || id >= m_cpuCount) {
            return 0.;
        }
        return 1.0 * m_cpuInfos[id].valueNew.doubleValue;
    }
    int syncCPUInfo(int msec) {
        LARGE_INTEGER time1, time2;
        QueryPerformanceCounter(&time1);

        Sleep(msec);

        QueryPerformanceCounter(&time2);
        loadAllInfo();

        m_deltaTime = time2.QuadPart - time1.QuadPart;

        return 0;
    }
    int getCPUCount() { return m_cpuCount; }

   private:
    int m_cpuCount;
    CPUInfo m_totalInfo;
    std::vector<CPUInfo> m_cpuInfos;
    ULONGLONG m_deltaTime;

    void loadAllInfo() {
        for (int i = 0; i < m_cpuCount; i++) {
            m_cpuInfos[i].valueOld = m_cpuInfos[i].valueNew;
            PdhCollectQueryData(m_cpuInfos[i].query);
            PdhGetFormattedCounterValue(m_cpuInfos[i].counter, PDH_FMT_DOUBLE,
                                        NULL, &m_cpuInfos[i].valueNew);
        }
    }
};