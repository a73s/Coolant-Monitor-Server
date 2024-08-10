cmake_minimum_required(VERSION 3.20)

project("Coolant Monitor Server"
	DESCRIPTION "Socket server for esp32 coolant monitor"
	LANGUAGES CXX
	)

file(GLOB SOURCE_FILES
	./src/*.cpp
	./src/*.h
	./src/*.hpp
	# ./src/**/*.cpp
	# ./src/**/*.h
	# ./src/**/*.hpp
	)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(CURSES REQUIRED IMPORTED_TARGET ncurses)
pkg_check_modules(ASIO REQUIRED IMPORTED_TARGET asio)

find_package(mdns_cpp QUIET)
if(NOT mdns_cpp_FOUND)
	message("mdns_cpp not found, installing locally")
	execute_process(
		COMMAND git submodule update --init -- mdns_cpp
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)
	add_subdirectory(mdns_cpp)
endif()

# ensure that you get release mode by default
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(serverMonitor ${SOURCE_FILES})
# target_link_libraries(serverMonitor PRIVATE mdns_cpp::mdns_cpp ncurses)
target_link_libraries(serverMonitor PRIVATE mdns_cpp::mdns_cpp PkgConfig::CURSES)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)
install(TARGETS serverMonitor DESTINATION "${CMAKE_INSTALL_BINDIR}")
