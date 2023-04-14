cmake_minimum_required(VERSION 3.0.0)
project(project2 VERSION 0.1.0)

# include(CTest)
# enable_testing()


# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR} )
set (CMAKE_CXX_STANDARD 17)
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

# set source and headers
SET (SRC_FILES common.hpp core.hpp core.cpp main.cpp)

# build 
add_executable(${PROJECT_NAME} ${SRC_FILES} )
