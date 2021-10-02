add_executable(gb_main_wx
		"main_wnd.cpp")

# find_package(gl3w CONFIG REQUIRED)
find_package(Magnum REQUIRED GL Shaders)
if(MAGNUM_TARGET_GLES AND NOT MAGNUM_TARGET_DESKTOP_GLES)
    find_package(Magnum REQUIRED EglContext)
elseif(CORRADE_TARGET_WINDOWS)
    find_package(Magnum REQUIRED WglContext)
elseif(CORRADE_TARGET_APPLE)
    find_package(Magnum REQUIRED CglContext)
elseif(CORRADE_TARGET_UNIX)
    find_package(Magnum REQUIRED GlxContext)
else()
    message(FATAL_ERROR "Magnum context creation is not supported on this platform")
endif()

set(wxWidgets_EXCLUDE_COMMON_LIBRARIES TRUE)
find_package(wxWidgets REQUIRED core base gl)
find_path(LIBNOP_INCLUDE_DIRS "nop/base/array.h")

target_include_directories(gb_main_wx
		PRIVATE ${LIBNOP_INCLUDE_DIRS}
		PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_compile_definitions(gb_main_wx
		PRIVATE ${wxWidgets_DEFINITIONS})
target_compile_options(gb_main_wx
		PRIVATE ${wxWidgets_CXX_FLAGS})
target_link_libraries(gb_main_wx
		gb_core
		util
		# unofficial::gl3w::gl3w
		Magnum::Magnum
		Magnum::GL
		Magnum::GLContext
		Magnum::Shaders
		nowide::nowide
		${wxWidgets_LIBRARIES})

if(WIN32)
	# To force WinMain Entrypoint for wxWidgets
	target_link_options(gb_main_wx
			PRIVATE "LINKER:/SUBSYSTEM:WINDOWS,/ENTRY:WinMainCRTStartup")

	# Dpi Awareness
	target_sources(gb_main_wx
			PRIVATE "resource.rc")
	target_compile_definitions(gb_main_wx
			PRIVATE wxUSE_DPI_AWARE_MANIFEST=1)

	set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)

	#copy WX DLLs to build directory
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