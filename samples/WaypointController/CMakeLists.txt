##################################################
#
# CMakeLists.txt - Root cmake file for SampleHighLevelController
#
# History:
#   * Original file - Dan Swain, 7/6/10
#   * Simplied significantly - DTS 7/14/10
#   
##################################################

# this is the most recent version of CMake
cmake_minimum_required (VERSION 2.8)

# name of the project - this has to come first
project(WaypointController)

# MADTRaC information
set(MT_ROOT $ENV{MT_ROOT} CACHE PATH "MADTraC root directory")
# sets variables related to MT
include(${MT_ROOT}/cmake/MT_Config.cmake)

# set the name of the built target
set(APP_NAME WaypointController)

# set this to a list of source and header files to be compiled
set(APP_SRC
  src/waypoint_controller.cc
  )

# adds src to the header search path
include_directories(src)

# sets up the build target
add_executable(${APP_NAME} ${APP_SRC})


# sets up the app to build correctly using the right libraries, etc
MT_robot_app(${APP_NAME})