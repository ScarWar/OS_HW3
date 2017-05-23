cmake_minimum_required(VERSION 3.7)
project(OS_HW3)

set(CMAKE_C_STANDARD 90)

set(SOURCE_FILES
        dispatcher.c
        #        counter.c
        )
#add_executable(OS_HW3 ${SOURCE_FILES})
#add_executable(counter.c ${SOURCE_FILES})
add_executable(dispatcher.c ${SOURCE_FILES})