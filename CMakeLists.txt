cmake_minimum_required(VERSION 3.5)

project("Coolant Monitor Server"
	DESCRIPTION "Socket server for esp32 coolant monitor"
	LANGUAGES CXX
	)

file(GLOB SOURCE_FILES
	./src/*.cpp
	./src/*.h
	./src/*.hpp
	)

add_subdirectory(mdns_cpp)

# find_package(asio REQUIRED)
# find_package(ncurses REQUIRED)

add_executable(serverMonitor ${SOURCE_FILES})
target_link_libraries(serverMonitor PRIVATE mdns_cpp::mdns_cpp)
target_link_libraries(serverMonitor PRIVATE ncurses)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -W -pedantic-errors -static -g)
add_link_options(-Wall -W -pedantic-errors -static -g)

if("${build_mode}" STREQUAL "Debug")
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
	add_compile_options(-fno-omit-frame-pointer)
	add_link_options(-fno-omit-frame-pointer)
else()
	add_compile_options("-O3")
	# add_link_options("")
endif()

include(GNUInstallDirs)
install(TARGETS serverMonitor DESTINATION "${CMAKE_INSTALL_BINDIR}")
