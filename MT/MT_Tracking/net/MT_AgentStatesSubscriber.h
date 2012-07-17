#ifndef MT_AGENTSTATESSUBSCRIBER_H_
#define MT_AGENTSTATESSUBSCRIBER_H_

#include "MT_Subscriber.h"
#include "MT_AgentStates.pb.h"
#include <boost/function.hpp>

class MT_AgentStatesSubscriber : public MT_Subscriber {
 public:
  MT_AgentStatesSubscriber(const std::string& host, const std::string& service);
  MT_AgentStates PopMostRecentReceivedMessage();
  MT_AgentStates PopOldestReceivedMessage();
 private:
    
  MT_AgentStates agent_states_;
};

#endif  // MT_AGENTSTATESSUBSCRIBER_H_
