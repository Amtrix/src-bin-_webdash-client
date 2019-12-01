cmake_minimum_required(VERSION 3.10)

set (EXTERNAL_LIB_PATH "$ENV{MYWORLD}/src/lib/external")
set (CMAKE_CXX_COMPILER /usr/bin/g++-9)
set (CMAKE_CXX_FLAGS "-std=c++1z -msse4.2 -Wall -Wextra -O3 -g -fopenmp -lstdc++fs")

###### Include directories
FIND_PACKAGE(Boost 1.65 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${EXTERNAL_LIB_PATH}/json/include)
include_directories(${EXTERNAL_LIB_PATH}/websocketpp)
include_directories($ENV{MYWORLD}/src/common)
include_directories($ENV{MYWORLD}/src/lib/webdash-executer/include)


###### *.cpp code to compile
list(APPEND ALL_CPP_FILES

)

###### Binaries to create
add_executable(webdash src/main.cpp)

###### Library paths
target_link_libraries(webdash -L"$ENV{MYWORLD}/app-persistent/lib")

###### Libraries
target_link_libraries(webdash Boost::filesystem)
target_link_libraries(webdash webdash-executer)