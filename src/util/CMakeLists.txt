add_library(util
		"file.cpp")

find_package(ZLIB REQUIRED)

target_link_libraries(util
	PRIVATE ZLIB::ZLIB)