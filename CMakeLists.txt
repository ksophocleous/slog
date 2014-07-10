cmake_minimum_required (VERSION 2.8)

set(libname "slog")
project(${libname} CXX)

set(version_major 0)
set(version_minor 8)
set(version_patch 5)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-std=c++11")
endif()

if(MSVC)
	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
	endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(SLOG_BUILD_TESTS "Build tests" ON)

set(libtype STATIC)

set(src
	"src/slog.cpp"
	"src/slog_logdevice_file.cpp"
	"src/slog_logdevice_custom_function.cpp"
	"src/slog_logdevice_console.cpp"
	)

set(hdr_public
	"include/slog/slog.h"
	"include/slog/slog_logdevice_custom_function.h"
	"include/slog/slog_logdevice_file.h"
	"include/slog/slog_logdevice_console.h"
	)

list(APPEND include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/include")
list(APPEND compile_defines "VERSION_MAJOR=${version_major}")
list(APPEND compile_defines "VERSION_MINOR=${version_minor}")
list(APPEND compile_defines "VERSION_PATCH=${version_patch}")

if (NOT TARGET ${libname})
	add_library(${libname} ${libtype} ${src} ${hdr_public})

	set_target_properties(${libname} PROPERTIES
		PUBLIC_HEADER "${hdr_public}"
		INCLUDE_DIRECTORIES "${include_dirs}"
		INTERFACE_INCLUDE_DIRECTORIES "${include_dirs}"
		VERSION "${version_major}.${version_minor}.${version_patch}"
		COMPILE_DEFINITIONS "${compile_defines}")

	set(testname "slog_tests")
	if(SLOG_BUILD_TESTS)
		add_executable(${testname} EXCLUDE_FROM_ALL "tests/tests.cpp")
		target_link_libraries(${testname} ${libname})
	endif()

	install(TARGETS ${libname}
		LIBRARY DESTINATION lib COMPONENT lib
		ARCHIVE DESTINATION lib COMPONENT lib
		RUNTIME DESTINATION bin COMPONENT bin
		PUBLIC_HEADER DESTINATION include/slog COMPONENT dev)
endif()