cmake_minimum_required (VERSION 2.8)

set(libname "slog")
project(${libname} CXX)

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

option(BUILD_TESTS "Build tests" ON)

set(libtype STATIC)

set(src
	"src/slog.cpp"
	"src/slog_logdevice_file.cpp")

set(hdr_public
	"include/slog/slog.h")

list(APPEND include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(${libname} ${libtype} ${src} ${hdr_public})

set_target_properties(${libname} PROPERTIES
	PUBLIC_HEADER "${hdr_public}"
	INCLUDE_DIRECTORIES "${include_dirs}")

if(BUILD_TESTS)
	add_executable(tests "tests/tests.cpp")
	set_target_properties(tests PROPERTIES
		INCLUDE_DIRECTORIES "${include_dirs}")
	target_link_libraries(tests ${libname})
endif()

install(TARGETS ${libname}
	LIBRARY DESTINATION lib COMPONENT lib
	ARCHIVE DESTINATION lib COMPONENT lib
	RUNTIME DESTINATION bin COMPONENT bin
	PUBLIC_HEADER DESTINATION include COMPONENT dev)