find_package(Boost 1.36.0 COMPONENTS system thread)
include_directories(
  ${Boost_INCLUDE_DIRS}
)
find_package(Protobuf REQUIRED)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS agent_states.proto)
message(${PROTO_SRCS})
include_directories(
  ${PROTOBUF_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src/net
  )

add_library(net
  agent_states_publisher.cc
  agent_states_subscriber.cc
  publisher.cc
  subscriber.cc
  ${PROTO_SRCS}
  )