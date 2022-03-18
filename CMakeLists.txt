cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 14)

# ZMQ Stuff
add_compile_definitions(ZMQ_STATIC)

include_directories(
		Sources/
		External/ZMQ/include/
)

link_directories(
		External/ZMQ/lib/
)

set(ALL_LIBS
		general
		libzmq.a
		ws2_32
		iphlpapi
		)

# Project Stuff
# Project 1
project(main)

set(project_headers
		#Sources/*.hpp
		)

set(project_sources
		#Sources/*.cpp
		Sources/${PROJECT_NAME}.cpp
		)

message("PROJECT_NAME=" ${PROJECT_NAME})
message("PROJECT_SOURCES=" ${project_sources})
message("PROJECT_HEADERS=" ${project_headers})
message("PROJECT_LIBS=" ${ALL_LIBS})

add_executable(${PROJECT_NAME} ${project_sources} ${project_headers})

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})
