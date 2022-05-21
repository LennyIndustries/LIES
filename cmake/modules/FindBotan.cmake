#find_path(BOTAN_INCLUDE_DIRS NAMES botan/botan.h DOC "The botan include directory")
#find_library(BOTAN_LIBRARIES NAMES botan botan-2 DOC"The botan library")
#include(FindPackageHandleStandardArgs)
#find_package_handle_standard_args(Botan REQUIRED_VARS BOTAN_LIBRARIES BOTAN_INCLUDE_DIRS)

#if (Botan_FOUND)
#    add_library(Botan::Botan UNKNOWN IMPORTED)
#    set_target_properties(Botan::Botan PROPERTIES IMPORTED_LOCATION "${BOTAN_LIBRARIES}" INTERFACE_INCLUDED_DIRECTORIES "${BOTAN_INCLUDE_DIRS}")
#endif ()

#mark_as_advanced(BOTAN_LIBRARIES BOTAN_INCLUDE_DIRS)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/External/Botan/")
#[[
#include(FindPkgConfig)
find_package(PkgConfig)

if(NOT BOTAN_FOUND)
    pkg_check_modules(BOTAN botan-2)
endif()

if(NOT BOTAN_FOUND)
    find_path(BOTAN_INCLUDE_DIRS NAMES botan/botan.h
            PATH_SUFFIXES botan-2
            DOC "The botan include directory")

    find_library(BOTAN_LIBRARIES NAMES botan botan-2 External Botan
            DOC "The botan library")

    # Use some standard module to handle the QUIETLY and REQUIRED arguments, and
    # set BOTAN_FOUND to TRUE if these two variables are set.
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(BOTAN REQUIRED_VARS BOTAN_LIBRARIES BOTAN_INCLUDE_DIRS)

    if(BOTAN_FOUND)
        set(BOTAN_LIBRARY ${BOTAN_LIBRARIES} CACHE INTERNAL "")
        set(BOTAN_INCLUDE_DIR ${BOTAN_INCLUDE_DIRS} CACHE INTERNAL "")
        set(BOTAN_FOUND ${BOTAN_FOUND} CACHE INTERNAL "")
    endif()
endif()

mark_as_advanced(BOTAN_INCLUDE_DIRS BOTAN_LIBRARIES)]]
#[[
find_package(PkgConfig REQUIRED)

if (NOT TARGET Botan::Botan)
    #pkg_check_modules(Botan IMPORTED_TARGET botan-2)
    #add_library(Botan::Botan ALIAS PkgConfig::Botan)
endif ()

if (NOT TARGET Botan::Botan)
    find_path(Botan_INCLUDE_DIRS NAMES botan/botan.h
            PATH_SUFFIXES botan-2
            DOC "The Botan include directory")

    find_library(Botan_LIBRARIES NAMES botan botan-2
            DOC "The Botan library")

    mark_as_advanced(Botan_INCLUDE_DIRS Botan_LIBRARIES)

    add_library(Botan::Botan IMPORTED UNKNOWN)
    set_target_properties(
            Botan::Botan
            PROPERTIES
            IMPORTED_LOCATION "${Botan_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${Botan_INCLUDE_DIRS}"
    )
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
        Botan
        REQUIRED_VARS Botan_LIBRARIES Botan_INCLUDE_DIRS
)]]

find_package(PkgConfig REQUIRED)

if (NOT TARGET Botan::Botan)
    pkg_check_modules(Botan IMPORTED_TARGET botan-2)
    if (TARGET PkgConfig::Botan)
        add_library(Botan::Botan ALIAS PkgConfig::Botan)
    endif ()
endif ()

if (NOT TARGET Botan::Botan)
    find_path(Botan_INCLUDE_DIRS NAMES botan/botan.h
            PATH_SUFFIXES botan-2
            DOC "The Botan include directory")

    find_library(Botan_LIBRARIES NAMES botan botan-2
            DOC "The Botan library")

    mark_as_advanced(Botan_INCLUDE_DIRS Botan_LIBRARIES)

    add_library(Botan::Botan IMPORTED UNKNOWN)
    set_target_properties(
            Botan::Botan
            PROPERTIES
            IMPORTED_LOCATION "${Botan_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${Botan_INCLUDE_DIRS}"
    )
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
        Botan
        REQUIRED_VARS Botan_LIBRARIES Botan_INCLUDE_DIRS
)