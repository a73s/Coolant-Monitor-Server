cmake_minimum_required(VERSION 3.5)

project("Coolant Monitor Server"
	DESCRIPTION "Socket server for esp32 coolant monitor"
	LANGUAGES CXX
	)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SOURCE_FILES
	./src/*.cpp
	./src/*.h
	./src/*.hpp
	)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -W -pedantic-errors -g -static -O0" )
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -W -pedantic-errors -s -static -O3")

# add_custom_target(asan-target PRIVATE)

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)
# add_compile_options(-fno-omit-frame-pointer)
# add_link_options(-fno-omit-frame-pointer)

add_subdirectory(mdns_cpp-color)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(serverMonitor ${SOURCE_FILES})
target_link_libraries(serverMonitor PRIVATE mdns_cpp::mdns_cpp)
