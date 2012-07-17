#include "MT_Subscriber.h"
#include "MT_AgentStates.pb.h"
#include "MT_AgentStatesSubscriber.h"
#include <boost/function.hpp>
#include <iostream>

MT_AgentStatesSubscriber::MT_AgentStatesSubscriber(const std::string& host, const std::string& service, boost::function<void (MT_AgentStates)> handle_new_message)
  : MT_Subscriber(host, service),
    handle_new_message_(handle_new_message) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
    }
  
void MT_AgentStatesSubscriber::HandleNewMessage(std::string inbound_data) {
  // Extract the data structure from the data just received.     
  if (!incoming_agent_states_.ParseFromString(inbound_data)) {
    // Unable to decode data.
    std::cerr << "Error parsing protocol buffer." << std::endl;
    return;
  }
  handle_new_message_(incoming_agent_states_);
}


