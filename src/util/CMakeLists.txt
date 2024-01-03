add_library(util
		"file.cpp")

find_package(ZLIB REQUIRED)

target_include_directories(gb_core
        PRIVATE "."
        PUBLIC "../")

target_link_libraries(util
	PRIVATE ZLIB::ZLIB
	PRIVATE nowide::nowide)