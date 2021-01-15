add_executable(gb_main_wx
		"main_wnd.cpp")

find_package(gl3w CONFIG REQUIRED)
find_package(wxWidgets REQUIRED gl core base OPTIONAL_COMPONENTS net)

target_include_directories(gb_main_wx
	PRIVATE "./libnop/include")

include(${wxWidgets_USE_FILE})
target_link_libraries(gb_main_wx
		gb_core
		util
		unofficial::gl3w::gl3w
		${wxWidgets_LIBRARIES})

if(MSVC)
	# To force WinMain Entrypoint
	target_link_options(gb_main_wx
		PRIVATE "LINKER:/SUBSYSTEM:WINDOWS,/ENTRY:WinMainCRTStartup")

	# Cmake apparently doesn't like copying DLLs, only import libs
	# it also refuses to link wxwidgets statically so we have to copy the DLLs manually

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(WXMSW_DLLS
			"${wxWidgets_LIB_DIR}/wxbase314ud_vc14x_x64.dll"
			"${wxWidgets_LIB_DIR}/wxmsw314ud_gl_vc14x_x64.dll"
			"${wxWidgets_LIB_DIR}/wxmsw314ud_core_vc14x_x64.dll")
	else()
		set(WXMSW_DLLS
			"${wxWidgets_LIB_DIR}/wxbase314u_vc14x_x64.dll"
			"${wxWidgets_LIB_DIR}/wxmsw314u_gl_vc14x_x64.dll"
			"${wxWidgets_LIB_DIR}/wxmsw314u_core_vc14x_x64.dll")
	endif()

	# Copy Command
	add_custom_command(TARGET gb_main_wx POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${WXMSW_DLLS}
			$<TARGET_FILE_DIR:gb_main_wx>
		COMMENT "Copying ${WXMSW_DLLS}")
endif()