add_library(gb_core
        "apu.cpp"
        "cart.cpp"
        "core.cpp"
        "cpu.cpp"
        "cpu_disassem.cpp"
        "joy.cpp"
        "io.cpp"
        "mem.cpp"
        "ppu.cpp")

target_include_directories(gb_core
        PUBLIC "../inc")

target_link_libraries(gb_core
        PRIVATE nowide::nowide)