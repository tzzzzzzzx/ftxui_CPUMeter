cmake_minimum_required(VERSION 3.11)

# set the project name
project(ftxui-cpumeter
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 17)

# fetchcontent test?
include(FetchContent)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v3.0.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/ftxui
)
FetchContent_MakeAvailable(ftxui)
# add executable
add_executable(cpuchk ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/CPUMonitor.cpp)
target_link_libraries(cpuchk
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)
target_include_directories(cpuchk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

