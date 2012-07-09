##################################################
#
# MT/MT_Tracking/CMakeLists.txt - cmake file for MT_Tracking
#
# History:
#	* Changed to include YASegmenter - Valeri Karpov, 7/7/2010
#   * Original file - Dan Swain, 6/10/10
#   
##################################################

# require cmake version 2.8 since it has a module
#  to find HDF5.  2.8 is available on mac via
#  macports/darwinports
cmake_minimum_required (VERSION 2.8)

set(module_name "MT_Tracking")

#######################################################
# Target: MT_Tracking.a
#######################################################
# sources
set(3rdparty_srcs
  ./3rdparty/cvBlobsLib/Blob.cpp
  ./3rdparty/cvBlobsLib/Blob.h
  ./3rdparty/cvBlobsLib/BlobContour.cpp
  ./3rdparty/cvBlobsLib/BlobContour.h
  ./3rdparty/cvBlobsLib/BlobOperators.cpp      
  ./3rdparty/cvBlobsLib/BlobOperators.h
  ./3rdparty/cvBlobsLib/BlobResult.cpp
  ./3rdparty/cvBlobsLib/BlobResult.h
  ./3rdparty/cvBlobsLib/BlobResult_t.h
  ./3rdparty/cvBlobsLib/ComponentLabeling.cpp
  ./3rdparty/cvBlobsLib/ComponentLabeling.h
  ./3rdparty/cvBlobsLib/BlobLibraryConfiguration.h
  ./3rdparty/libhungarian-0.3/hungarian.c
  ./3rdparty/libhungarian-0.3/hungarian.h  
  ./3rdparty/libhungarian-0.3/makeprob.c
)
set(base_srcs
  ./base/MT_TrackerBase.cpp      ./base/MT_TrackerBase.h
  ./base/MT_TrackerFrameBase.cpp ./base/MT_TrackerFrameBase.h)
set(capture_srcs
  ./capture/MT_Capture.cpp             ./capture/MT_Capture.h
  ./capture/MT_Capture_Interfaces.cpp  ./capture/MT_Capture_Interfaces.h
  ./capture/MT_AVTCameraDialog.cpp	   ./capture/MT_AVTCameraDialog.h)
set(cv_srcs
  ./cv/MT_BlobExtras.cpp             ./cv/MT_BlobExtras.h
  ./cv/MT_HungarianMatcher.cpp       ./cv/MT_HungarianMatcher.h
  ./cv/MT_MakeBackgroundFrame.cpp      ./cv/MT_MakeBackgroundFrame.h
  ./cv/GSThresholder.cpp             ./cv/GSThresholder.h
  ./cv/MT_CalibrationDataFile.cpp    ./cv/MT_CalibrationDataFile.h) 
set(dialogs_srcs
  ./dialogs/MT_CreateBackgroundDialog.cpp
  ./dialogs/MT_CreateBackgroundDialog.h
  ./dialogs/MT_CameraSlaveFrame.cpp
  ./dialogs/MT_CameraSlaveFrame.h)
set(trackers_srcs
  ./trackers/GY/GYBlobs.cpp       ./trackers/GY/GYBlobs.h
  ./trackers/GY/GYSegmenter.cpp   ./trackers/GY/GYSegmenter.h
  ./trackers/GY/MixGaussians.cpp  ./trackers/GY/MixGaussians.h
  ./trackers/GY/GYBlobber.cpp     ./trackers/GY/GYBlobber.h
  ./trackers/YA/YABlobber.cpp     ./trackers/YA/YABlobber.h
  ./trackers/YA/YASegmenter.cpp     ./trackers/YA/YASegmenter.h
  ./trackers/DS/DSGYA_Segmenter.cpp ./trackers/DS/DSGYA_Segmenter.h
  ./trackers/DS/DSGYBlobber.cpp    ./trackers/DS/DSGYBlobber.h)

include_directories(
  ${Boost_INCLUDE_DIRS}
)

set(proto_files
  ./net/MT_AgentStates.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS "${proto_files}")
message(${PROTO_SRCS})
include_directories(
  ${PROTOBUF_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/MT/${module_name}
  )

set(net_srcs
  ./net/MT_AgentStatesPublisher.cc  ./net/MT_AgentStatesPublisher.h
  ./net/MT_AgentStatesSubscriber.cc ./net/MT_AgentStatesSubscriber.h
  ./net/MT_Publisher.cc             ./net/MT_Publisher.h
  ./net/MT_Subscriber.cc            ./net/MT_Subscriber.h  
)

set(protobuf_srcs
  ${PROTO_SRCS}                     ${PROTO_HDRS}
)

include_directories(${WX_INCLUDE})
add_definitions(${WX_DEFS})

set(module_srcs ${3rdparty_srcs} ${base_srcs} ${capture_srcs} ${cv_srcs} ${dialogs_srcs} ${trackers_srcs} ${net_srcs})

if(WITH_ARTK)
  include_directories(${ARTK_INCLUDE})
  add_definitions(-DMT_HAVE_ARTOOLKIT)
endif(WITH_ARTK)

# tell cmake this is a static library built from the
#   given sources
if(BUILD_SHARED)
  add_library(${module_name} SHARED ${module_srcs} ${protobuf_srcs})
  target_link_libraries(${module_name} MT_Core)
  target_link_libraries(${module_name} MT_GUI)
  target_link_libraries(${module_name} ${WX_EXTRA_LIBS})
  target_link_libraries(${module_name} ${MT_PROTOBUF_LIB})
  target_link_libraries(${module_name} ${MT_BOOST_LIB})
else(BUILD_SHARED)  
  add_library(${module_name} STATIC ${module_srcs} ${protobuf_srcs})
endif(BUILD_SHARED)

if(HAVE_OPENCV_FRAMEWORK)
  target_link_libraries(${module_name} ${OPENCV_FRAMEWORK})
endif()

# copies the header files to the "include" directory
# if(NOT MSVC)
#   get_directory_property(defs DIRECTORY . COMPILE_DEFINITIONS)
#   add_custom_command(TARGET MT_Tracking POST_BUILD
#     COMMAND echo ARGS "\"${defs}\">defs.txt"
#     COMMAND ${CMAKE_SOURCE_DIR}/misc/build_scripts/posix_cmake_target_script.sh ARGS "MT_Tracking" ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
#     )
# endif(NOT MSVC)

# if(MSVC)
#   string(REPLACE "/" "\\" ARG2 ${CMAKE_SOURCE_DIR})
#   string(REPLACE "/" "\\" ARG3 ${CMAKE_BINARY_DIR})
#   add_custom_command(TARGET MT_Tracking POST_BUILD
#     COMMAND ${CMAKE_SOURCE_DIR}/misc/build_scripts/MSVC_cmake_post_build.bat ARGS "MT_Tracking" ${ARG2} ${ARG3})
# endif(MSVC)    

include(${CMAKE_SOURCE_DIR}/cmake/MT_BuildFunctions.cmake)

foreach(proto_file ${proto_files})
  string(REPLACE ".proto" ".pb.h" transposed_protobuf_src ${proto_file})
  set(transposed_protobuf_srcs "${transposed_protobuf_srcs};${transposed_protobuf_src}")
endforeach()
MT_MAKE_MODULE_HEADER(. "${module_name}" "${module_srcs};${transposed_protobuf_srcs}")

# Protobuf sources get dumped in the binary directory, so they have to be treated differently than the rest of the module sources
foreach(proto_file ${proto_files})
  get_filename_component(tree_part "${proto_file}" PATH)
  get_filename_component(name_part "${proto_file}" NAME_WE)

  # using a custom command here makes sure the headers get copied whenever they change
  add_custom_command(TARGET "${module_name}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/include/MT/${module_name}/${tree_part}"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/MT/${module_name}/${name_part}.pb.h" "${CMAKE_BINARY_DIR}/include/MT/${module_name}/${tree_part}/"
    )
endforeach()
MT_COPY_MODULE_HEADERS("${module_name}" "${module_srcs}" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/include/MT/${module_name}")