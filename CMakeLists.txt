cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 17)

# ZMQ Stuff
add_compile_definitions(ZMQ_STATIC)

include_directories(
		Sources/
		External/ZMQ/include/
		External/OpenSSL/include/
		External/Botan/include/
		External/QT/include/
)

link_directories(
		External/ZMQ/lib/
		External/OpenSSL/lib/
		External/Botan/lib/
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
# Project 1 - Server
project(server)

set(project_headers
		#Sources/*.hpp
		Sources/include/lilog.hpp
		)

set(project_sources
		#Sources/*.cpp
		Sources/${PROJECT_NAME}.cpp
		Sources/lib/lilog.cpp
		)

message("PROJECT_NAME=" ${PROJECT_NAME})
message("PROJECT_SOURCES=" ${project_sources})
message("PROJECT_HEADERS=" ${project_headers})
message("PROJECT_LIBS=" ${ALL_LIBS})

add_executable(${PROJECT_NAME} ${project_sources} ${project_headers})

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})

# Project 2 - Client
project(client)

set(project_headers
		#Sources/*.hpp
		Sources/include/lilog.hpp
		)

set(project_sources
		#Sources/*.cpp
		Sources/${PROJECT_NAME}.cpp
		Sources/lib/lilog.cpp
		)

message("PROJECT_NAME=" ${PROJECT_NAME})
message("PROJECT_SOURCES=" ${project_sources})
message("PROJECT_HEADERS=" ${project_headers})
message("PROJECT_LIBS=" ${ALL_LIBS})

add_executable(${PROJECT_NAME} ${project_sources} ${project_headers})

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})
