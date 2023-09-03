#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <queue>
#include <string>

#include "CPUMonitor.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/flexbox_config.hpp"
#include "ftxui/dom/node.hpp"
#include "ftxui/screen/color.hpp"
#include "ftxui/screen/color_info.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/terminal.hpp"

static CPUMonitor cpu_monitor;

int CAL_BETWEEN(int c, int a, int b) { return c > b ? (b) : (c < a ? a : c); }

class CPUTotalUsageGraph {
    // Referenced to examples/graph.cpp
   public:
    CPUTotalUsageGraph() { history_info.push_back(0.0); }
    std::vector<int> operator()(int width, int height) {
        while (width < history_info.size()) {
            history_info.erase(history_info.begin());
        }
        std::vector<int> output(width);
        for (int i = 0; i < width; ++i) {
            int id = i + history_info.size() - width;
            if (id < 0) id = 0;
            double v = history_info[id];
            v = v / 100 * height;
            output[i] = CAL_BETWEEN(static_cast<int>(v), 0, height);
        }
        return output;
    }
    std::vector<double> history_info;
};

int main() {
    using namespace ftxui;
    Elements percpu_gauges;
    CPUTotalUsageGraph totalcpu_graph;
    std::string reset_position;
    while (1) {
        int sync_state = cpu_monitor.syncCPUInfo(1000);
        if (sync_state) return sync_state;  // detect error
        totalcpu_graph.history_info.push_back(cpu_monitor.getCPUUsageTotal());

        // PERCPU_UI
        percpu_gauges.clear();
        for (int i = 0; i < cpu_monitor.getCPUCount(); i++) {
            std::string alloc_percent =
                std::to_string(cpu_monitor.getCPUUsage(i));
            percpu_gauges.push_back(
                hbox({text("CPU " +
                           (i < 10    ? std::string("  ")
                            : i < 100 ? std::string(" ")
                                      : std::string("")) +
                           std::to_string(i) + " "),
                      gauge(cpu_monitor.getCPUUsage(i) / 100),
                      text(" " +
                           alloc_percent.substr(0, alloc_percent.length() - 5) +
                           " %")}));
        }
        auto percpu_ui = vbox(percpu_gauges);

        // TOTALCPU_UI
        std::string alloc_percent =
            std::to_string(cpu_monitor.getCPUUsageTotal());
        auto totalcpu_ui = vbox({
            text("Total CPU Usage: " +
                 alloc_percent.substr(0, alloc_percent.length() - 5) + " %"),
            graph(std::ref(totalcpu_graph)),
        });

        auto document = hbox({
            std::move(totalcpu_ui) | border | color(Color::Red) | flex,
            std::move(percpu_ui) | flex | border,
        });

        auto screen =
            Screen::Create(Dimension::Full(), Dimension::Fit(document));
        Render(screen, document);
        std::cout << reset_position;
        screen.Print();
        reset_position = screen.ResetPosition();
    }

    return 0;
}
