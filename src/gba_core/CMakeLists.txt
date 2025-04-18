add_library(gba_core
        "cart.cpp"
        "cpu.cpp"
        "io.cpp")
target_include_directories(gba_core
        PUBLIC "../")
target_link_libraries(gba_core
        nowide::nowide)
