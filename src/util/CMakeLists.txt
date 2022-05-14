add_library(util
		"file.cpp")

find_package(ZLIB REQUIRED)

target_include_directories(util
	PUBLIC "../inc")

target_link_libraries(util
	PRIVATE ZLIB::ZLIB
	PRIVATE nowide::nowide)