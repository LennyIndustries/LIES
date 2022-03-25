cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 14)

# ZMQ Stuff
add_compile_definitions(ZMQ_STATIC)

include_directories(
		Sources/
		External/ZMQ/include/
		External/OpenSSL/include/
		External/Botal/include/
		External/QT/include/
)

link_directories(
		External/ZMQ/lib/
		External/OpenSSL/lib/
		External/Botal/lib/
		External/QT/lib/
)

set(ALL_LIBS
		general
		libzmq.a
		ws2_32
		iphlpapi
		libcrypto.a
		libssl.a
		libbotan-2.a
		libQt6Core.a
		)

# Project Stuff
# Project 1
project(main)

set(project_headers
		#Sources/*.hpp
		Sources/lilog.hpp
		)

set(project_sources
		#Sources/*.cpp
		Sources/${PROJECT_NAME}.cpp
		Sources/lilog.cpp
		)

message("PROJECT_NAME=" ${PROJECT_NAME})
message("PROJECT_SOURCES=" ${project_sources})
message("PROJECT_HEADERS=" ${project_headers})
message("PROJECT_LIBS=" ${ALL_LIBS})

add_executable(${PROJECT_NAME} ${project_sources} ${project_headers})

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})
