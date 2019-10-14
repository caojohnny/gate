cmake_minimum_required(VERSION 3.13)
project(gatecli VERSION 1.0 LANGUAGES C)

include("${CMAKE_SOURCE_DIR}/cmake/DownloadCspice.cmake")

find_package(gate 1.0 REQUIRED)
find_package(gatesnm 1.0 REQUIRED)

add_executable(gatecli
        main.c
        clihandler.c clihandler.h
        commands.c commands.h
        dispatcher.c dispatcher.h
        options.c options.h
        util.c util.h
        )
target_link_libraries(gatecli
        PRIVATE gate
        PRIVATE gatesnm)
