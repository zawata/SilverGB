add_library(gb_core
        "apu.cpp"
        "cart.cpp"
        "core.cpp"
        "cpu.cpp"
        "cpu_disassem.cpp"
        "joy.cpp"
        "io.cpp"
        "initial_state.cpp"
        "mem.cpp"
        "ppu.cpp")

target_include_directories(gb_core
        PUBLIC "../inc")

if(BUILD_WITH_ASAN)
        target_compile_options(gb_core
                PUBLIC "-fsanitize=address"
                PUBLIC "-fno-omit-frame-pointer")
        target_link_options(gb_core
                PUBLIC "-fsanitize=address")
endif()

target_link_libraries(gb_core
        PRIVATE nowide::nowide)