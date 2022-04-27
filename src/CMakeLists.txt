project(avionicsbay LANGUAGES CXX DESCRIPTION "Avionics Bay - X-Plane manager for data files")
cmake_minimum_required(VERSION 3.10)

if (GITHUB)

    execute_process(
      COMMAND git log -1 --format=%H
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
endif(GITHUB)

# Set C++14 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SOURCES api.cpp
            cifp_parser.cpp
            data_file_reader.cpp
            plugin.cpp
            triangulator.cpp
            xpdata.cpp
            utilities/logger.cpp
            wmm/GeomagnetismLibrary.cpp
            wmm_interface.cpp)

add_library(avionicsbay SHARED ${SOURCES})


# Platform specific
if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --no-gnu-unique -pthread")
endif (UNIX AND NOT APPLE)


if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif (MINGW)


if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif (APPLE)

set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/../../64/")
install(TARGETS avionicsbay DESTINATION .)

