add_executable(gdxiterbench
    iterbench.cpp
)

add_executable(gdxtransformbench
    transformbench.cpp
)

target_link_libraries(gdxiterbench PRIVATE gdxcore gdxalgo benchmark::benchmark)
target_link_libraries(gdxtransformbench PRIVATE gdxcore gdxalgo benchmark::benchmark)
