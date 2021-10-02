add_library(gb_core
		"apu.cpp"
		"cart.cpp"
		"core.cpp"
		"cpu.cpp"
		"cpu_disassem.cpp"
		"input.cpp"
		"io.cpp"
		"ppu.cpp")

target_link_libraries(gb_core
	PRIVATE nowide::nowide)