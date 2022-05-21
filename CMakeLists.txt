cmake_minimum_required(VERSION 3.21)

message("CMAKE_VERSION = " ${CMAKE_VERSION})

set(CMAKE_CXX_STANDARD 17)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
#find_package(Botan REQUIRED)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
#find_package(BOTAN REQUIRED)

#set(THREADS_PREFER_PTHREAD_FLAG ON)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
#find_package(Botan REQUIRED)

#if(BOTAN_FOUND)
#    message("Found Botan: " ${BOTAN_LIBRARIES} " : " ${BOTAN_INCLUDE_DIRS})
#endif()

#set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/External/Boost/)
#find_package(Boost REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(Botan REQUIRED)

# ZMQ Stuff
add_compile_definitions(ZMQ_STATIC)

include_directories(
        Sources/
        External/ZMQ/include/
#        External/OpenSSL/include/
#        External/QT/include/
        External/Botan/include/
#        ${BOTAN_INCLUDE_DIRS}
#        External/Boost/include/
#        ${Boost_INCLUDE_DIR}
)

link_directories(
        External/ZMQ/lib/
#        External/OpenSSL/lib/
        External/Botan/lib/
#        External/QT/lib/
#        External/Boost/lib/
)

set(ALL_LIBS
        general
        libzmq.a
        ws2_32
        iphlpapi
#        libcrypto.a
#        libssl.a
#        libQt6Core.a
        libbotan-2.a
#        ${BOTAN_LIBRARIES}
#        ${Boost_LIBRARIES}
        )

# Project Stuff
# Project 1 - Server
project(server)#LANGUAGES CXX C
#[[
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/release/0.17/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            EXPECTED_HASH SHA256=3bef79da16c2e031dc429e1dac87a08b9226418b300ce004cc125a82687baeef
            TLS_VERIFY ON)
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_configure(REQUIRES botan/2.19.1
        GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        OPTIONS botan:shared=False
        SETTINGS ${settings})

find_package(botan)]]

set(project_headers
        #Sources/?include?/*.hpp
        Sources/include/lilog.hpp
        Sources/include/inputHandler.hpp
        Sources/include/encrypt.hpp
        Sources/include/decrypt.hpp
        Sources/include/cryptLib.hpp
        Sources/include/connectionHandler.hpp
        )

set(project_sources
        #Sources/?lib?/*.cpp
        Sources/${PROJECT_NAME}.cpp
        Sources/lib/lilog.cpp
        Sources/lib/inputHandler.cpp
        Sources/lib/encrypt.cpp
        Sources/lib/decrypt.cpp
        Sources/lib/cryptLib.cpp
        Sources/lib/connectionHandler.cpp
        )

message("PROJECT_NAME=" ${PROJECT_NAME})
message("PROJECT_SOURCES=" ${project_sources})
message("PROJECT_HEADERS=" ${project_headers})
message("PROJECT_LIBS=" ${ALL_LIBS})

add_executable(${PROJECT_NAME} ${project_sources} ${project_headers})

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS} Botan::Botan) #PRIVATE Botan::Botan

# Project 2 - Client
project(client)

set(project_headers
        #Sources/?include?/*.hpp
        Sources/include/lilog.hpp
        Sources/include/inputHandler.hpp
        Sources/include/encrypt.hpp
        Sources/include/decrypt.hpp
        Sources/include/cryptLib.hpp
        Sources/include/connectionHandler.hpp
        )

set(project_sources
        #Sources/?lib?/*.cpp
        Sources/${PROJECT_NAME}.cpp
        Sources/lib/lilog.cpp
        Sources/lib/inputHandler.cpp
        Sources/lib/encrypt.cpp
        Sources/lib/decrypt.cpp
        Sources/lib/cryptLib.cpp
        Sources/lib/connectionHandler.cpp
        )

message("PROJECT_NAME=" ${PROJECT_NAME})
message("PROJECT_SOURCES=" ${project_sources})
message("PROJECT_HEADERS=" ${project_headers})
message("PROJECT_LIBS=" ${ALL_LIBS})

add_executable(${PROJECT_NAME} ${project_sources} ${project_headers})

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})
