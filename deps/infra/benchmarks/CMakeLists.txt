find_package(benchmark REQUIRED)

add_executable(strsplitbench
    stringsplit.cpp
)

target_link_libraries(strsplitbench PRIVATE infra benchmark::benchmark)
