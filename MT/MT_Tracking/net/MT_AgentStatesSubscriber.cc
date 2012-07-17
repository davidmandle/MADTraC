#include "MT_Subscriber.h"
#include "MT_AgentStates.pb.h"
#include "MT_AgentStatesSubscriber.h"
#include <boost/function.hpp>
#include <iostream>

MT_AgentStatesSubscriber::MT_AgentStatesSubscriber(const std::string& host, const std::string& service)
  : MT_Subscriber(host, service) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
    }

MT_AgentStates MT_AgentStatesSubscriber::PopMostRecentReceivedMessage() {
  agent_states_.ParseFromString(MT_Subscriber::PopMostRecentReceivedMessage());
  return agent_states_;
}

MT_AgentStates MT_AgentStatesSubscriber::PopOldestReceivedMessage() {
  agent_states_.ParseFromString(MT_Subscriber::PopOldestReceivedMessage());
  return agent_states_;
}


