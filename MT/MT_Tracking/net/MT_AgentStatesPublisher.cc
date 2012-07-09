#include "MT_Publisher.h"
#include "MT_AgentStatesPublisher.h"
#include "MT_AgentStates.pb.h"

MT_AgentStatesPublisher::MT_AgentStatesPublisher(unsigned short port) : MT_Publisher(port) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
  }

  bool MT_AgentStatesPublisher::Publish(MT_AgentStates agent_states) {
    // Serialize the data
    std::string outbound_data;
    if (!agent_states.SerializeToString(&outbound_data)) {
      // Failed to serialize data
      return false;
    }

    return MT_Publisher::Publish(outbound_data);
  }

