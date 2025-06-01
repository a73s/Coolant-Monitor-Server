cmake_minimum_required(VERSION 3.20)

project("Coolant Monitor Server"
	DESCRIPTION "Socket server for esp32 coolant monitor"
	LANGUAGES CXX
)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
	${PROJECT_SOURCE_DIR}/src/*.cpp
	${PROJECT_SOURCE_DIR}/src/*.h
	${PROJECT_SOURCE_DIR}/src/*.hpp
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(CURSES REQUIRED IMPORTED_TARGET ncurses)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

# find mdns_cpp dependency or install it as a git submodule
find_package(mdns_cpp QUIET)
if(NOT mdns_cpp_FOUND)
	message(STATUS "mdns_cpp not found, installing locally")
	execute_process(
		COMMAND git submodule update --init -- extern/mdns_cpp
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)
	add_subdirectory(extern/mdns_cpp)
else()
	message(STATUS "mdns_cpp found")
endif()

# ensure that you get release mode by default
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -fsigned-char")
# set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(serverMonitor ${SOURCE_FILES})
target_link_libraries(serverMonitor PRIVATE mdns_cpp::mdns_cpp PkgConfig::CURSES PkgConfig::CURL PkgConfig::GTKMM)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)
install(TARGETS serverMonitor DESTINATION "${CMAKE_INSTALL_BINDIR}")
