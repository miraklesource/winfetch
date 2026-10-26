cmake_minimum_required(VERSION 3.10)

project(winfetch C)

add_executable(winfetch
    src/main.c
    src/util.c
    src/system_info.c
)

target_link_libraries(winfetch PRIVATE
    advapi32
    powrprof
    dxgi
    uuid
)
