cmake_minimum_required(VERSION 3.10)
project(ECF_GP_scheduling)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "-g")

include_directories(include)
include_directories(include/Nodes)
include_directories(periodic_tasks_simulator/include)
include_directories(ECF/ECF)

add_executable(ECF_GP_scheduling main.cpp
    		src/TaskEvalOp.cpp
			src/RTO.cpp
			src/RLP.cpp
			src/EDL.cpp)


add_subdirectory( periodic_tasks_simulator )
add_subdirectory( ECF )
target_link_libraries( ECF_GP_scheduling periodic_simulator )
target_link_libraries(ECF_GP_scheduling ECF)
